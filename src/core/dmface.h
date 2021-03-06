/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2011-2012  Christian Urich
 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef DMFACE_H
#define DMFACE_H
#include <dmcompilersettings.h>

#include <vector>
#include <string>

namespace DM {
class Component;
class Node;

/** @ingroup DynaMind-Core
  * @brief Provides a Face object. A Face just contains references to a nodes
  *
  * A Face is defined by a vector of Nodes. The order in the vector describes the face.
  * Faces are derived from the Component class. Therefore faces are identified by an UUID and can hold an
  * unlimeted number of Attributes. Faces only contain references to nodes stored in the system. Faces are always closed
  * therefore the start node != end node
  * @TODO orientation
  */
class DM_HELPER_DLL_EXPORT Face :  public Component
{
public:
	/** @brief destructor */
	~Face();

	/** @brief Creates a new Face. A face is defined by a vector of references (uuid's) to existing nodes */
	Face(const std::vector<Node*>& nodes);

	/** @brief Create a copy of the face also including the Component (Attributes and uuid)*/
	Face(const Face& e);

	/** @brief return Type */
	Components getType() const;

	/** @brief return vector of nodes defining the face  */
	std::vector<Node*> getNodePointers() const;

	/** @brief  Creates a pointer to a cloned Face object, including Attributes and uuid*/
	Component * clone();

	/** @brief Returns a vector of holes  */
	const std::vector<Face*> getHolePointers() const;

	/** @brief Add hole */
	void addHole(const std::vector<Node*>& hole);
	void addHole(Face* hole);

	/** @brief this function is neccessary to make the copy constructor a usefull */
	void setNodes(const std::vector<Node*>& nodes);
	/** @brief deletes all holes */
	void clearHoles();

	/** @brief exports the component to the db, it can afterwards deleted safly */
	void _moveToDb();
private:
	/** @brief return table name */
	QString getTableName();

	std::vector<Node*> _nodes;
	std::vector<Face*> _holes;
};
typedef std::map<std::string, DM::Face*> FaceMap;
}
#endif // DMFACE_H
