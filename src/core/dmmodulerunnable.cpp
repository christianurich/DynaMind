/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2011  Christian Urich

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
#include "dmmodulerunnable.h"

#include <QThreadPool>
#include <QElapsedTimer>
#include <dmmodule.h>
#include <dmgroup.h>
#include <dmlogger.h>
#include <dmsimulation.h>

DM::ModuleRunnable::ModuleRunnable(DM::Module * m)
{
    this-> m = m;
    this->setAutoDelete(true);


}

void DM::ModuleRunnable::run() {




    if (!m->checkPreviousModuleUnchanged())
        m->setExecuted(false);
    if (!m->isExecuted() || m->isGroup()) {


        m->updateParameter();
        m->init();
        //Called twice since the user can change data in the init method!
        m->updateParameter();
        if (!m->checkIfAllSystemsAreSet())
            return;

        if (!m->getSimulation()->isVirtualRun() || m->isGroup()) {
            if (m->getSimulation()->getSimulationStatus() == DM::SIM_OK) {
                DM::Logger(DM::Debug) << this->m->getUuid()<< "Run";
                QElapsedTimer timer;
                timer.start();
                DM::Logger(DM::Standard) << "Start\t"  << m->getClassName() << " "  << m->getName()<< " " << m->getUuid() << " Counter " << m->getInternalCounter();
                m->run();
                m->setExecuted(true);

                DM::Logger(DM::Standard) << "Success\t" << m->getClassName() << " "  << m->getName()<< " " << m->getUuid() << " Counter " << m->getInternalCounter()  <<  "\t time " <<  (double) timer.elapsed()/1000;
            }
        }
        m->postRun();


    }



    DM::Group * g = m->getGroup();
    if (g!=0 && !m->isGroup())
        g->finishedModule(this->m);
}
