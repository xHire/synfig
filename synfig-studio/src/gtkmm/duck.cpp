/* === S Y N F I G ========================================================= */
/*!	\file duck.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "duck.h"
#include <ETL/misc>

#include <synfig/valuenode_bline.h>
#include <synfig/valuenode_blinecalctangent.h>
#include <synfig/valuenode_blinecalcvertex.h>
#include <synfig/valuenode_blinecalcwidth.h>
#include <synfig/valuenode_composite.h>

#include "general.h"
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

int studio::Duck::duck_count(0);

struct _DuckCounter
{
	static int counter;
	~_DuckCounter()
	{
		if(counter)
			synfig::error("%d ducks not yet deleted!",counter);
	}
} _duck_counter;

int _DuckCounter::counter(0);


/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Duck::Duck():
	rotations(synfig::Angle::deg(0)),
	origin(0,0),
	scalar(1),
	editable(false),
	radius_(false),
	tangent_(false),
	hover_(false),
	ignore_(false)
{ duck_count++; _DuckCounter::counter++; }

Duck::Duck(const synfig::Point &point):
	type_(TYPE_NONE),
	point(point),
	rotations(synfig::Angle::deg(0)),
	origin(0,0),
	scalar(1),
	guid_(0),
	editable(false),
	radius_(false),
	tangent_(false),
	hover_(false),
	ignore_(false)
{ duck_count++; _DuckCounter::counter++;}

Duck::Duck(const synfig::Point &point,const synfig::Point &origin):
	point(point),
	rotations(synfig::Angle::deg(0)),
	origin(origin),
	scalar(1),
	guid_(0),
	editable(false),
	radius_(true),
	tangent_(false),
	hover_(false),
	ignore_(false)
{ duck_count++; _DuckCounter::counter++;}

Duck::~Duck() { duck_count--; _DuckCounter::counter--;}

synfig::GUID
Duck::get_data_guid()const
{
	if(value_desc_.is_value_node())
		return value_desc_.get_value_node()->get_guid();
	return synfig::GUID::hasher(get_name());
}

void
Duck::set_name(const synfig::String &x)
{
	name=x;
	if(guid_==synfig::GUID::zero())
	{
		guid_=synfig::GUID::hasher(name);
	}
}


bool
Duck::operator==(const Duck &rhs)const
{
	if(this==&rhs)
		return true;
	return
		name==rhs.name &&
		scalar==rhs.scalar &&
		type_==rhs.type_ &&
		transform_stack_.size()==rhs.transform_stack_.size();
		//true;
		//(origin_duck?*origin_duck==*rhs.origin_duck:origin==rhs.origin) &&
		//(shared_point?*shared_point==*rhs.shared_point:point==rhs.point) ;
}

synfig::Point
Duck::get_trans_point()const
{
	return transform_stack_.perform(get_sub_trans_point());
}

void
Duck::set_trans_point(const synfig::Point &x)
{
	set_sub_trans_point(transform_stack_.unperform(x));
}

void
Duck::set_trans_point(const synfig::Point &x, const synfig::Time &time)
{
	set_sub_trans_point(transform_stack_.unperform(x), time);
}

//! Sets the origin point.
void
Duck::set_origin(const synfig::Point &x)
{
	origin=x; origin_duck=0;
}

//! Sets the origin point as another duck
void
Duck::set_origin(const etl::handle<Duck> &x)
{
	origin_duck=x;
}

//! Retrieves the origin location
synfig::Point
Duck::get_origin()const
{
	return origin_duck?origin_duck->get_point():origin;
}

//! Retrieves the origin duck
const etl::handle<Duck> &
Duck::get_origin_duck() const
{
	return origin_duck;
}

//! Retrieves the origin location
synfig::Point
Duck::get_trans_origin()const
{
	return transform_stack_.perform(get_sub_trans_origin());
}

synfig::Point
Duck::get_sub_trans_point()const
{
	return get_point()*get_scalar()+get_sub_trans_origin();
}

void
Duck::set_sub_trans_point(const synfig::Point &x, const synfig::Time &time)
{
	set_point((x-get_sub_trans_origin())/get_scalar());
	if (get_type() == Duck::TYPE_TANGENT ||
		get_type() == Duck::TYPE_ANGLE)
	{
		Angle old_angle = get_point().angle();
		Angle change = get_point().angle() - old_angle;
		while (change < Angle::deg(-180)) change += Angle::deg(360);
		while (change > Angle::deg(180)) change -= Angle::deg(360);
		int old_halves = round_to_int(Angle::deg(rotations).get()/180);
		rotations += change;
		int new_halves = round_to_int(Angle::deg(rotations).get()/180);
		if (old_halves != new_halves &&
			(new_halves > 1 || new_halves < -1 ||
			 old_halves > 1 || old_halves < -1))
			synfig::info("rotation: %.2f turns", new_halves/2.0);
	} else if(get_type() == Duck::TYPE_VERTEX || get_type() == Duck::TYPE_POSITION)
	{

		ValueNode_BLineCalcVertex::Handle bline_vertex;
		ValueNode_Composite::Handle composite;

		if ((bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(get_value_desc().get_value_node())) ||
			((composite = ValueNode_Composite::Handle::cast_dynamic(get_value_desc().get_value_node())) &&
			 composite->get_type() == ValueBase::TYPE_BLINEPOINT &&
			 (bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(composite->get_link("point")))))
		{
			synfig::Point closest_point = get_point();
			synfig::Real radius = 0.0;
			ValueNode_BLine::Handle bline = ValueNode_BLine::Handle::cast_dynamic(bline_vertex->get_link(bline_vertex->get_link_index_from_name("bline")));
			synfig::find_closest_point(
				(*bline)(time),
				get_point(),
				radius,
				bline->get_loop(),
				&closest_point);
			set_point(closest_point);
		}
	}
}

//! Updates width and tangent ducks that change when the origin moves
void
Duck::update(const synfig::Time &time)
{
	if((get_type() == Duck::TYPE_TANGENT || get_type() == Duck::TYPE_WIDTH) && origin_duck)
	{
		ValueNode_BLineCalcVertex::Handle bline_vertex;
		ValueNode_Composite::Handle composite;
		if ((bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(origin_duck->get_value_desc().get_value_node())) ||
			((composite = ValueNode_Composite::Handle::cast_dynamic(origin_duck->get_value_desc().get_value_node())) &&
			 composite->get_type() == ValueBase::TYPE_BLINEPOINT &&
			 (bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(composite->get_link("point")))))
		{
			synfig::Real radius = 0.0;
			ValueNode_BLine::Handle bline(ValueNode_BLine::Handle::cast_dynamic(bline_vertex->get_link(bline_vertex->get_link_index_from_name("bline"))));
			Real amount = synfig::find_closest_point((*bline)(time), origin_duck->get_point(), radius, bline->get_loop());

			int vertex_amount_index(bline_vertex->get_link_index_from_name("amount"));
			ValueNode::Handle vertex_amount_value_node(bline_vertex->get_link(vertex_amount_index));


			if (ValueNode_BLineCalcTangent::Handle bline_tangent = ValueNode_BLineCalcTangent::Handle::cast_dynamic(get_value_desc().get_value_node()))
			{
				switch (bline_tangent->get_type())
				{
				case ValueBase::TYPE_ANGLE:
				{
					Angle angle((*bline_tangent)(time, amount).get(Angle()));
					set_point(Point(Angle::cos(angle).get(), Angle::sin(angle).get()));
					break;
				}
				case ValueBase::TYPE_REAL:
					set_point(Point((*bline_tangent)(time, amount).get(Real()), 0));
					break;
				case ValueBase::TYPE_VECTOR:
					set_point((*bline_tangent)(time, amount).get(Vector()));
					break;
				default:
					break;
				}
			}
			else if (ValueNode_BLineCalcWidth::Handle bline_width = ValueNode_BLineCalcWidth::Handle::cast_dynamic(get_value_desc().get_value_node()))
					set_point(Point((*bline_width)(time, amount).get(Real()), 0));
		}

	}
}


void
Duck::set_sub_trans_point(const synfig::Point &x)
{
	set_point((x-get_sub_trans_origin())/get_scalar());
	if (get_type() == Duck::TYPE_TANGENT ||
		get_type() == Duck::TYPE_ANGLE)
	{
		Angle old_angle = get_point().angle();
		Angle change = get_point().angle() - old_angle;
		while (change < Angle::deg(-180)) change += Angle::deg(360);
		while (change > Angle::deg(180)) change -= Angle::deg(360);
		int old_halves = round_to_int(Angle::deg(rotations).get()/180);
		rotations += change;
		int new_halves = round_to_int(Angle::deg(rotations).get()/180);
		if (old_halves != new_halves &&
			(new_halves > 1 || new_halves < -1 ||
			 old_halves > 1 || old_halves < -1))
			synfig::info("rotation: %.2f turns", new_halves/2.0);
	}
}

synfig::Point
Duck::get_sub_trans_origin()const
{
	return origin_duck?origin_duck->get_sub_trans_point():origin;
}

#ifdef _DEBUG
synfig::String
Duck::type_name(Type id)
{
	String ret;

	if (id & TYPE_POSITION) { if (!ret.empty()) ret += ", "; ret += "position"; }
	if (id & TYPE_TANGENT ) { if (!ret.empty()) ret += ", "; ret += "tangent" ; }
	if (id & TYPE_RADIUS  ) { if (!ret.empty()) ret += ", "; ret += "radius"  ; }
	if (id & TYPE_WIDTH	  ) { if (!ret.empty()) ret += ", "; ret += "width"   ; }
	if (id & TYPE_ANGLE	  ) { if (!ret.empty()) ret += ", "; ret += "angle"   ; }
	if (id & TYPE_VERTEX  ) { if (!ret.empty()) ret += ", "; ret += "vertex"  ; }

	if (ret.empty())
		ret = "none";

	return ret;
}
#endif	// _DEBUG