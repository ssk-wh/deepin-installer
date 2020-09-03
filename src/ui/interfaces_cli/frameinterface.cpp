#include "frameinterface.h"
#include <QThread>
#include <QTimer>

namespace installer {

FrameInterface::FrameInterface(FrameInterface* parent)
    : m_parent(parent),
      m_currState(FRAME_STATE_NOT_START),
      m_currTask(0),
      m_cangetchild(false)
{

}

FrameInterface::~FrameInterface()
{

}


void FrameInterface::addChildFrame(FrameInterface* childFrame)
{
    m_childFrame.push_back(childFrame);
}

void FrameInterface::start()
{
    auto currThreadId = QThread::currentThreadId();
    while (true) {
        QString currFrame = getFrameName();
        if (m_currState == FRAME_STATE_ABORT
                || m_currState == FRAME_STATE_FINISH
                || m_currState == FRAME_STATE_ERROR) {
            return;
        }

        if (!init()) {
            m_currState = FRAME_STATE_ERROR;
            return;
        }
        m_currState = FRAME_STATE_RUNNING;
        handle();

        qDebug() << "currFrame=" << getFrameName() << "m_currTask=" << m_currTask;
        if (m_currTask < m_childFrame.size()) {

            FrameInterface* frameInterface = m_childFrame[m_currTask];
            if ((frameInterface)) {
                qDebug() << "currFrame=" << frameInterface->getFrameName() << "m_currTask=" << m_currTask;
                frameInterface->start();
            }
        }
    }
}

void FrameInterface::setCanGetChild(bool iscan)
{
//    for (int i = 0; i < m_childFrame.size(); i++) {
//        m_childFrame.at(i)->setCanGetChild(iscan);
//    }
    m_cangetchild = iscan;
}

FrameInterface *FrameInterface::getCurrentChild()
{
    if((m_currTask >= 0) && (m_currTask < m_childFrame.size()))
    {
        if(m_childFrame.at(m_currTask) != nullptr) {
            if(m_cangetchild) {
                return m_childFrame.at(m_currTask)->getCurrentChild();
            } else {
                return m_childFrame.at(m_currTask);
            }
        }
        else {
            return m_childFrame.at(m_currTask);
        }
    } else {
        return nullptr;
    }
}


bool FrameInterface::init()
{
    if (m_currState == FRAME_STATE_NOT_START) {
           m_currState = FRAME_STATE_RUNNING;
    }

    return true;
}

void FrameInterface::initConnection()
{
    auto itChildFrame = m_childFrame.begin();
       for ( ; itChildFrame != m_childFrame.end(); ++itChildFrame) {
           connect((*itChildFrame)->getPrivate(), &FrameInterfacePrivate::next, this, [=](){
               if ((m_currTask < m_childFrame.size()) && m_childFrame[m_currTask]->getFrameState() == FRAME_STATE_RUNNING) {
                   m_childFrame[m_currTask]->setFrameState(FRAME_STATE_FINISH);
                   //m_childFrame[m_currTask]->getPrivate()->hide();
                   m_childFrame[m_currTask]->hide();
                   m_currTask++;
                   Q_EMIT update(m_childFrame[m_currTask]->getAbout());
                   if (m_currTask >= m_childFrame.size()) {
                       return;
                   }
                   if (m_childFrame[m_currTask]->getFrameState() != FRAME_STATE_NOT_START) {
                       m_childFrame[m_currTask]->setFrameState(FRAME_STATE_RUNNING);
                   }
                   if (m_childFrame[m_currTask]->shouldDisplay()) {
                        m_childFrame[m_currTask]->show();
                   } else {
                       m_childFrame[m_currTask]->setFrameState(FRAME_STATE_FINISH);
                       //m_childFrame[m_currTask]->getPrivate()->hide();
                       m_childFrame[m_currTask]->hide();
                       m_currTask++;
                       if (m_currTask >= m_childFrame.size()) {
                           return;
                       }
                       if (m_childFrame[m_currTask]->getFrameState() != FRAME_STATE_NOT_START) {
                           m_childFrame[m_currTask]->setFrameState(FRAME_STATE_RUNNING);
                       }
                       m_childFrame[m_currTask]->show();
                   }

                   qDebug() << "frame = " << m_childFrame[m_currTask]->getFrameName();
                   qDebug() << "m_currTask" << m_currTask;
                   qDebug() << "m_childFrame.size = " << m_childFrame.size();
               }

           });
           connect((*itChildFrame)->getPrivate(), &FrameInterfacePrivate::back, [this](){
               if (m_currTask > 0 && m_childFrame[m_currTask]->getFrameState() == FRAME_STATE_RUNNING) {
                   m_childFrame[m_currTask]->setFrameState(FRAME_STATE_ABORT);
                   //m_childFrame[m_currTask]->getPrivate()->hide();
                   m_childFrame[m_currTask]->hide();
                   m_currTask--;
                   m_childFrame[m_currTask]->setFrameState(FRAME_STATE_RUNNING);
                   if (m_childFrame[m_currTask]->shouldDisplay()) {
                        m_childFrame[m_currTask]->show();
                   } else {
                       m_childFrame[m_currTask]->setFrameState(FRAME_STATE_ABORT);
                       //m_childFrame[m_currTask]->getPrivate()->hide();
                       m_childFrame[m_currTask]->hide();
                       m_currTask--;
                       m_childFrame[m_currTask]->setFrameState(FRAME_STATE_RUNNING);
                       m_childFrame[m_currTask]->show();
                   }

                   Q_EMIT update(m_childFrame[m_currTask]->getAbout());
                   qDebug() << "m_currTask" << m_currTask;
               }else if (m_currTask == 0 && m_childFrame.size() > 0) {
                   if (m_parent != nullptr) {
                       m_childFrame[m_currTask]->setFrameState(FRAME_STATE_ABORT);
                       m_currState = FRAME_STATE_RUNNING;
                       //m_parent->m_currTask--;
                   }
                   //m_childFrame[m_currTask]->getPrivate()->hide();
               }
               else {
                   m_currState = FRAME_STATE_FINISH;
               }

           });
       }
}

void FrameInterface::show()
{
    init();
    m_private->updateTs();
    m_private->show();
}

void FrameInterface::hide()
{
    hideAllChild();
    m_private->hide();
}

void FrameInterface::showChild(int index)
{
    if(index < m_childFrame.size()) {
        m_childFrame[index]->show();
    }
}

void FrameInterface::hideChild(int index)
{
    if(index < m_childFrame.size()) {
        m_childFrame[index]->hide();
    }
}

void FrameInterface::hideAllChild()
{
    for(int i = 0; i < m_childFrame.size(); i++){
        //m_childFrame[i]->setFrameState(FRAME_STATE_ABORT);
        m_childFrame[i]->hide();
    }
}

bool FrameInterface::shouldDisplay() const
{
    return true;
}


void FrameInterface::abort()
{
    m_currState = FRAME_STATE_ABORT;
}


}
