/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @author  Michael Mair <abroxos@gmail.com>
 * @author  Markus Sengthaler <m.sengthaler@gmail.com>
 * @version 1.0
 * @section LICENSE
 * This file is part of DynaMind
 *
 * Copyright (C) 2011  Christian Urich, Michael Mair, Markus Sengthaler

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

#include <QtCore>
#include <dmcomponent.h>
#include <dmnode.h>
#include <dmedge.h>
#include <dmrasterdata.h>
#include <dmattribute.h>
#include <dmsystem.h>
#include <assert.h>
#include <dmlogger.h>

#include "dmface.h"
#include <QUuid>

#include <dmdbconnector.h>
#include <QSqlQuery>

using namespace DM;



Component::Component()
{
//#ifndef GDAL
    DBConnector::getInstance();
    uuid = QUuid::createUuid();

    currentSys = NULL;
    isInserted = false;
    mutex = new QMutex(QMutex::Recursive);
//#endif


    DBConnector::getInstance();
#ifdef GDAL
	ogrFeature = NULL;
#endif
}

Component::Component(bool b)
{
    DBConnector::getInstance();
    uuid = QUuid::createUuid();

    currentSys = NULL;
    isInserted = false;
    mutex = new QMutex(QMutex::Recursive);
}

void Component::CopyFrom(const Component &c, bool successor)
{
    uuid = QUuid::createUuid();

    if(!successor)
    {
        foreach(Attribute* a, c.ownedattributes)
        {
            Attribute* newa = new Attribute(*a);
            ownedattributes.push_back(newa);
            newa->setOwner(this);
        }
    }
    else
		ownedattributes = c.ownedattributes;
}

#ifdef GDAL
void Component::initFeature()
{
	QMutexLocker ml(mutex);
	if (ogrFeature != NULL)
		return;
	if (this->currentSys == NULL)
		return;
	Logger(Debug) << "Init Feature";
	ogrFeature = OGRFeature::CreateFeature(this->currentSys->getComponentLayer()->GetLayerDefn());
	Logger(Debug) << "Done Feature";
}
#endif

Component::Component(const Component& c)
{
    currentSys = NULL;
    CopyFrom(c);
    mutex = new QMutex(QMutex::Recursive);
}

Component::Component(const Component& c, bool bInherited)
{
    currentSys = NULL;
    CopyFrom(c);
    mutex = new QMutex(QMutex::Recursive);
}

Component::~Component()
{
    mutex->lock();
    foreach(Attribute* a, ownedattributes)
        if(a->GetOwner() == this)
            delete a;

    ownedattributes.clear();
    // if this class is not of type component, nothing will happen
#ifndef GDAL
	SQLDelete();
	QMutexLocker ml(mutex);

    mutex->unlock();

    delete mutex;
#endif
}

Component& Component::operator=(const Component& other)
{
    QMutexLocker ml(mutex);

    if(this != &other)
    for (std::vector<Attribute*>::const_iterator it = other.ownedattributes.begin();
        it != other.ownedattributes.end(); ++it)
            this->addAttribute(*it);

    return *this;
}

std::string Component::getUUID()
{
    Attribute* a = this->getAttribute(UUID_ATTRIBUTE_NAME);
    std::string name = a->getString();
    if(name == "")
    {
        QMutexLocker ml(mutex);

        name = QUuid::createUuid().toString().toStdString();
        a->setString(name);
        //if(this->currentSys && currentSys != this)	// avoid self referencing
        //	currentSys->componentNameMap[name] = this;
    }
    return name;
}
QUuid Component::getQUUID() const
{
    return uuid;
}

DM::Components Component::getType() const
{
    return DM::COMPONENT;
}
QString Component::getTableName()
{
    return "components";
}

bool Component::addAttribute(const std::string& name, double val)
{
#ifndef GDAL
    QMutexLocker ml(mutex);

    if (Attribute* a = getExistingAttribute(name))
    {
        a->setDouble(val);
        return true;
    }

    return this->addAttribute(new Attribute(name, val));
#endif

#ifdef GDAL
	if (this->ogrFeature->GetDefnRef() == NULL) {
		Logger(Debug) << "Error";
		return false;
	}
	if (this->ogrFeature->GetDefnRef()->GetFieldIndex(name.c_str()) == -1) {
		OGRFieldDefn * oField = new OGRFieldDefn(name.c_str(), OFTReal);
		this->ogrFeature->GetDefnRef()->AddFieldDefn(oField);
	}
	Logger(Debug) << this->ogrFeature->GetDefnRef()->GetFieldIndex(name.c_str());

	return true;
#endif
}

bool Component::addAttribute(const std::string& name, const std::string& val)
{
    QMutexLocker ml(mutex);

    if (Attribute* a = getExistingAttribute(name))
    {
        a->setString(val);
        return true;
    }

    return this->addAttribute(new Attribute(name, val));
}

bool Component::addAttribute(const Attribute &newattribute)
{
    QMutexLocker ml(mutex);

    if (Attribute* a = getExistingAttribute(newattribute.getName()))
    {
        a->Change(newattribute);
        return true;
    }

    Attribute * a = new Attribute(newattribute);

    ownedattributes.push_back(a);
    a->setOwner(this);

    return true;
}

bool Component::addAttribute(Attribute *pAttribute)
{
	QMutexLocker ml(mutex);

	removeAttribute(pAttribute->getName());

	ownedattributes.push_back(pAttribute);
	pAttribute->setOwner(this);
	return true;
}

Attribute* Component::addAttribute(const std::string& name, Attribute::AttributeType type)
{
	QMutexLocker ml(mutex);

	removeAttribute(name);
	Attribute* a = new Attribute(name, type);
	ownedattributes.push_back(a);
	a->setOwner(this);
	return a;
}

bool Component::changeAttribute(const Attribute &newattribute)
{
    QMutexLocker ml(mutex);

    getAttribute(newattribute.getName())->Change(newattribute);
    return true;
}

bool Component::changeAttribute(const std::string& s, double val)
{
    QMutexLocker ml(mutex);

	getAttribute(s)->setDouble(val);
    return true;
}

bool Component::changeAttribute(const std::string& s, const std::string& val)
{
    QMutexLocker ml(mutex);

    getAttribute(s)->setString(val);
    return true;
}

bool Component::removeAttribute(const std::string& name)
{
    QMutexLocker ml(mutex);

    for (std::vector<Attribute*>::iterator  it = ownedattributes.begin(); it != ownedattributes.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            if ((*it)->GetOwner() == this)
                delete *it;

            ownedattributes.erase(it);
            return true;
        }
    }

    return false;
}

Attribute* Component::getAttribute(const std::string& name)
{
    if (name.empty())
        return NULL;

    QMutexLocker ml(mutex);

#ifndef GDAL
    std::vector<Attribute*>::iterator it;
    for (it = ownedattributes.begin(); it != ownedattributes.end(); ++it)
        if ((*it)->getName() == name)
            break;

    if (it == ownedattributes.end())
	{
		/*
		Attribute::AttributeType type = Attribute::NOTYPE;
		// get type
		if (this->currentSys)
		{
			mforeach(const System::ViewCache& vc, this->currentSys->viewCaches)
			{
				if (vc.view.hasAttribute(name))
				{
					Logger(Warning) << "extracting type of attribute '" << name
									<< "' from view '" << vc.view.getName() << "'";
					type = vc.view.getAttributeType(name);
					break;
				}
			}
		}
		*/

		// create new attribute with default type: DOUBLE
		Attribute* a = new Attribute(name, Attribute::DOUBLE);
		a->setOwner(this);
		ownedattributes.push_back(a);

        return a;
    }
    else if((*it)->GetOwner() != this)
	{
        // successor copy
        Attribute* a = new Attribute(**it);
        ownedattributes.erase(it);
        ownedattributes.push_back(a);
        a->setOwner(this);
        return a;
    }

    return *it;
#endif

#ifdef GDAL
	//DM::Attribute * attr = new DM::Attribute();
	//attr->setDouble(this->ogrFeature->GetFieldAsDouble(name.c_str()));
	//return attr;
#endif
}

const std::vector<Attribute*>& Component::getAllAttributes()
{
    // clone all attributes
    for (std::vector<Attribute*>::iterator it = ownedattributes.begin(); it != ownedattributes.end(); ++it)
	{
        if ((*it)->GetOwner() != this)
        {
            *it = new Attribute(**it);
            (*it)->setOwner(this);
        }
    }

    return ownedattributes;
}

Component* Component::clone()
{
    return new Component(*this);
}

System * Component::getCurrentSystem() const
{
    return this->currentSys;
}

void Component::SetOwner(Component *owner)
{
    QMutexLocker ml(mutex);

    currentSys = owner->getCurrentSystem();
#ifdef GDAL
	this->initFeature();
#endif
}

void Component::_moveToDb()
{
    // mutex will cause a crash
    mutex->lock();

    if(!isInserted)
    {
        DBConnector::getInstance()->Insert(	"components", uuid,
            "owner", (QVariant)currentSys->getQUUID().toByteArray());
    }
    else
    {
        DBConnector::getInstance()->Update(	"components", uuid,
            "owner", currentSys->getQUUID().toByteArray());
        isInserted = false;
    }
    _moveAttributesToDb();
    mutex->unlock();
    delete this;
}

void Component::_moveAttributesToDb()
{
    for (std::vector<Attribute*>::iterator it = ownedattributes.begin(); it != ownedattributes.end(); ++it)
        Attribute::_MoveAttribute(*it);
    ownedattributes.clear();
}

void Component::SQLDelete()
{
	QMutexLocker ml(mutex);

    if(isInserted)
    {
        isInserted = false;
        DBConnector::getInstance()->Delete(getTableName(), uuid);
    }
}

bool Component::hasAttribute(const std::string& name) const
{
    for (std::vector<Attribute*>::const_iterator it = ownedattributes.begin(); it != ownedattributes.end(); ++it)
        if ((*it)->getName() == name)
            return true;

    return false;
}

Attribute* Component::getExistingAttribute(const std::string& name) const
{
    for (std::vector<Attribute*>::const_iterator it = ownedattributes.begin(); it != ownedattributes.end(); ++it)
        if ((*it)->getName() == name)
            return *it;

    return NULL;
}

#ifdef GDAL
	OGRFeature * Component::getOGRFeature()
	{
		return ogrFeature;
	}

#endif
