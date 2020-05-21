#include "install_component_frame.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_progressbar.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_checkbox.h"
#include "ui/ncurses_widgets/ncurses_checkbox_list.h"
#include "ui/delegates/componentinstallmanager.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"

#include <QScopedPointer>

#include "cursesp.h"
#include "cursesm.h"


namespace installer {


InstallComponentFramePrivate::InstallComponentFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_tiltleLabel(nullptr),
      m_firstSubTiltleLabel(nullptr),
      m_secondSubTiltleLabel(nullptr),
      m_basicenvironmentlist(nullptr),
      m_extrachoiceslist(nullptr),
      m_selectallextra(nullptr),
      m_isshow(false)
{
    initUI();
    initConnection();
}

InstallComponentFramePrivate::~InstallComponentFramePrivate()
{
    if(m_basicenvironmentlist != nullptr) {
        delete m_basicenvironmentlist;
        m_basicenvironmentlist = nullptr;
    }

    if(m_extrachoiceslist != nullptr) {
        delete m_extrachoiceslist;
        m_extrachoiceslist = nullptr;
    }
}

void InstallComponentFramePrivate::initUI()
{
    setBackground(NcursesUtil::getInstance()->dialog_attr());
    FrameInterfacePrivate::initUI();

    QString testtiltle     = QObject::tr("Please chose the module you need");
    QString testfirststr   = QObject::tr("Basic Environment");
    QString testsecondstr  = QObject::tr("Add-Ons for Selected Environment");
    m_tiltleLabel          = new NcursesLabel(this, testtiltle.toUtf8().data(), 1, testtiltle.length(), begy() + 2,  begx() + (width() / 2) - (testtiltle.length() / 2));
    m_firstSubTiltleLabel  = new NcursesLabel(this, testfirststr.toUtf8().data(), 1, testfirststr.length(), begy() + 5, begx() + 2 + ((width() / 2 - 2) / 2) - (testfirststr.length() / 2));
    m_secondSubTiltleLabel = new NcursesLabel(this, testsecondstr.toUtf8().data(), 1, testsecondstr.length(), begy() + 5, begx() + width() / 2 + (width() / 2 - 2) / 2 - testsecondstr.length() / 2);

    m_tiltleLabel->setFocusEnabled(false);
    m_firstSubTiltleLabel->setFocusEnabled(false);
    m_secondSubTiltleLabel->setFocusEnabled(false);

    m_basicenvironmentlist = new NcursesCheckBoxList(this, height() - 15, width() / 2 - 2, begy() + 7,  begx() + 2);
    m_basicenvironmentlist->setFocus(true);
    m_basicenvironmentlist->setListType(NcursesCheckBoxList::BASICENVIRONMENT);
    m_basicenvironmentlist->setSingleSelect(true);
    m_basicenvironmentlist->setRealSelect(true);

    m_extrachoiceslist = new NcursesCheckBoxList(this, height() - 15, width() / 2 - 2, begy() + 7,  begx() + width() / 2);
    m_extrachoiceslist->setFocus(true);
    m_extrachoiceslist->setListType(NcursesCheckBoxList::EXTRACHOICES);
    m_extrachoiceslist->setSingleSelect(false);
    m_extrachoiceslist->setRealSelect(false);

    m_selectallextra = new NcursesCheckBox(this, QObject::tr("select all"), 1, width() / 2 - 2, begy() + height() - 7,  begx() + width() / 2  + 1);
}

void InstallComponentFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    printTitle(QObject::tr("Select Software"), width());

    QString testtiltle     = QObject::tr("Please chose the module you need");
    QString testfirststr   = QObject::tr("Basic Environment");
    QString testsecondstr  = QObject::tr("Add-Ons for Selected Environment");
    m_tiltleLabel->erase();
    m_firstSubTiltleLabel->erase();
    m_secondSubTiltleLabel->erase();
    if(installer::ReadLocale() == "zh_CN") {
        m_tiltleLabel->resizew(1, testtiltle.length() * 2);
        m_firstSubTiltleLabel->resizew(1, testfirststr.length() * 2);
        m_secondSubTiltleLabel->resizew(1, testsecondstr.length() * 2);
        m_selectallextra->setText("", QObject::tr("select all"), true);
    } else {
        m_tiltleLabel->resizew(1, testtiltle.length());
        m_firstSubTiltleLabel->resizew(1, testfirststr.length());
        m_secondSubTiltleLabel->resizew(1, testsecondstr.length());
        m_selectallextra->setText("", QObject::tr("select all"), false);
    }
    m_tiltleLabel->setText(testtiltle.toUtf8().data());
    m_firstSubTiltleLabel->setText(testfirststr.toUtf8().data());
    m_secondSubTiltleLabel->setText(testsecondstr.toUtf8().data());

    FrameInterfacePrivate::updateTs();

    initInfoList();
}

void InstallComponentFramePrivate::initConnection()
{
    connect(m_basicenvironmentlist, SIGNAL(signal_KeyTriger(int, int)), this, SLOT(slot_KeyTriger(int, int)));
    connect(m_extrachoiceslist, SIGNAL(signal_KeyTriger(int, int)), this, SLOT(slot_KeyTriger(int, int)));
    connect(m_selectallextra, SIGNAL(signal_SelectChange(bool)), this, SLOT(slot_SelectChange(bool)));
}

bool InstallComponentFramePrivate::validate()
{
    writeInfoList();
    return true;
}

void InstallComponentFramePrivate::show()
{
    if(!m_isshow){
        NCursesWindowBase::show();
        m_isshow = true;
    }
}

void InstallComponentFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void InstallComponentFramePrivate::initInfoList()
{
    ComponentInstallManager* manager = ComponentInstallManager::Instance();
    QList<QSharedPointer<ComponentStruct>> serverList = manager->list();

    m_basicenvironmentinfolist.clear();
    for (auto it = serverList.cbegin(); it != serverList.cend(); ++it) {
        QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(*it);
        if(tsPair.first.compare("")) {
            m_basicenvironmentinfolist.push_back(tsPair);
        }

        QVector<ComponentInfo> testcomponentinfovector;
        QList<QSharedPointer<ComponentInfo>> testextra = (*it)->extra();
        foreach(QSharedPointer<ComponentInfo> testinfo, testextra) {
            ComponentInfo testcomponentinfo;
            testcomponentinfo.Selected    = testinfo->Selected;
            testcomponentinfo.Id          = testinfo->Id;
            testcomponentinfo.PackageList = testinfo->PackageList;
            testcomponentinfovector.push_back(testcomponentinfo);
        }
        if(testcomponentinfovector.size() > 0) {
            m_componentInfomap.insert(tsPair.first, testcomponentinfovector);
        }
    }

    if (m_basicenvironmentinfolist.size() > 0) {
        QString teststr = m_basicenvironmentinfolist.at(0).first;
        m_extrachoicesinfolist.clear();
        foreach(ComponentInfo testinfo, m_componentInfomap[teststr])
        {
            QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(testinfo.Id);
            m_extrachoicesinfolist.push_back(tsPair);
        }

        bool testiswchar = false;
        if(installer::ReadLocale() == "zh_CN") {
            testiswchar = true;
        } else {
            testiswchar = false;
        }
        m_basicenvironmentlist->setList(m_basicenvironmentinfolist, testiswchar);
        m_extrachoiceslist->setList(m_extrachoicesinfolist, testiswchar);
    }
}

void InstallComponentFramePrivate::writeInfoList()
{
    WriteComponentPackages("");
    WriteComponentUninstallPackages("");

    ComponentInstallManager* manager = ComponentInstallManager::Instance();
    QList<QSharedPointer<ComponentStruct>> serverList = manager->list();

    for (auto it = serverList.cbegin(); it != serverList.cend(); ++it) {
        QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(*it);

        if(!tsPair.first.compare(m_basicenvironmentlist->getCurrentTitle())) {
            //QList<QSharedPointer<ComponentInfo>> testextra = (*it)->extra();
            QStringList testselects = m_extrachoiceslist->getSelectItems();
            //foreach(QSharedPointer<ComponentInfo> testinfo, (*it)->extra()) {
            for(int i = 0; i < (*it)->extra().size(); i++) {
                QPair<QString, QString> tsPair_temp = ComponentInstallManager::Instance()->updateTs((*it)->extra().at(i)->Id);
                if(testselects.indexOf(tsPair_temp.first) != -1) {
                    //testinfo->Selected = true;
                    (*it)->extra()[i]->Selected = true;
                }
            }

            const QStringList installPackages = manager->packageListByComponentStruct(*it);
            if (!installPackages.isEmpty()) {
                WriteComponentPackages(installPackages.join(" "));
            }

            const QStringList uninstallPackages = manager->uninstallPackageListByComponentStruct(*it, false);
            if (!uninstallPackages.isEmpty()) {
                WriteComponentUninstallPackages(uninstallPackages.join(" "));
            }

            break;
        }
    }
}


void InstallComponentFramePrivate::onKeyPress(int keyCode)
{

}


void InstallComponentFramePrivate::slot_KeyTriger(int keycode, int listtype)
{
    switch (keycode) {
    case KEY_UP:
        if(listtype == NcursesCheckBoxList::BASICENVIRONMENT) {
            QVector<QPair<QString, QString>> testinfolist;
            QString teststr = m_basicenvironmentlist->getCurrentTitle();
            foreach(ComponentInfo testinfo, m_componentInfomap[teststr])
            {
                QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(testinfo.Id);
                testinfolist.push_back(tsPair);
                //testinfolist.append(testinfo.Id);
            }
            m_extrachoiceslist->setList(testinfolist);
            m_extrachoiceslist->show();

            m_basicenvironmentlist->refresh();
            m_extrachoiceslist->refresh();
            refresh();
        }
        break;
    case KEY_DOWN:
        if(listtype == NcursesCheckBoxList::BASICENVIRONMENT) {
            QVector<QPair<QString, QString>> testinfolist;
            QString teststr = m_basicenvironmentlist->getCurrentTitle();
            foreach(ComponentInfo testinfo, m_componentInfomap[teststr])
            {
                QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(testinfo.Id);
                testinfolist.push_back(tsPair);
                //testinfolist.append(testinfo.Id);
            }
            m_extrachoiceslist->setList(testinfolist);
            m_extrachoiceslist->show();

            m_basicenvironmentlist->refresh();
            m_extrachoiceslist->refresh();
            refresh();
        }
        break;

    case 32:
        if(listtype == NcursesCheckBoxList::EXTRACHOICES) {

        }
        break;
    default:
        break;
    }
}

void InstallComponentFramePrivate::slot_SelectChange(bool select)
{
    m_extrachoiceslist->selectAll(select);
}


void InstallComponentFramePrivate::layout()
{
    m_tiltleLabel->mvwin(begy() + 2,  begx() + (width() / 2) - (m_tiltleLabel->width() / 2));
    m_firstSubTiltleLabel->mvwin(begy() + 5, begx() + 2 + ((width() / 2 - 2) / 2) - m_firstSubTiltleLabel->width() / 2);
    m_secondSubTiltleLabel->mvwin(begy() + 5, begx() + width() / 2 + (width() / 2 - 2) / 2 - m_secondSubTiltleLabel->width() / 2);

    /*m_basicenvironmentlist->show();
    m_extrachoiceslist->show();

    m_basicenvironmentlist->refresh();
    m_extrachoiceslist->refresh();
    this->refresh();*/
}

ComponentFrame::ComponentFrame(FrameInterface *parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new InstallComponentFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
}

ComponentFrame::~ComponentFrame()
{

}

bool ComponentFrame::init()
{
    Q_D(InstallComponentFrame);
    if (m_currState == FRAME_STATE_NOT_START) {
        d->initInfoList();
        m_private->layout();
    }
    return true;
}

QString ComponentFrame::getFrameName()
{
    return QObject::tr("SelectSoftwareFrame");
}

bool ComponentFrame::handle()
{
    return true;
}


}
