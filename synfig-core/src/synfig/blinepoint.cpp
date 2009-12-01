/* === S Y N F I G ========================================================= */
/*!	\file blinepoint.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "blinepoint.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
synfig::BLinePoint::reverse()
{
	std::swap(tangent_[0],tangent_[1]);
	tangent_[0]=-tangent_[0];
	tangent_[1]=-tangent_[1];
}

void
synfig::BLinePoint::normalize(int i)
{
	//i is the number of the tangent to modify
	//other tangent is (1-i)
	split_tangent_=(!link_radius_ && !link_theta_);
	if (link_radius_ && link_theta_)
		tangent_[i]=tangent_[1-i];
    else if (link_radius_)
		tangent_[i]=tangent_[i].norm()*tangent_[1-i].mag();
    else if (link_theta_)
		tangent_[i]=tangent_[1-i].norm()*tangent_[i].mag();

}