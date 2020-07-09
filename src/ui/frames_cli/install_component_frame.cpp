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
      m_isshow(false),
      m_localeString("")
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

    QString testtiltle     = ::QObject::tr("Select the components according to your needs.");
    QString testfirststr   = ::QObject::tr("Basic Environment");
    QString testsecondstr  = ::QObject::tr("Add-Ons");
    m_tiltleLabel          = new NcursesLabel(this, testtiltle.toUtf8().data(), 1, testtiltle.length(), begy() + 2,  begx() + (width() / 2) - (testtiltle.length() / 2));
    m_firstSubTiltleLabel  = new NcursesLabel(this, testfirststr.toUtf8().data(), 1, testfirststr.length(), begy() + 5, begx() + 2 + ((width() / 2 - 2) / 2) - (testfirststr.length() / 2));
    m_secondSubTiltleLabel = new NcursesLabel(this, testsecondstr.toUtf8().data(), 1, testsecondstr.length(), begy() + 5, begx() + width() / 2 + (width() / 2 - 2) / 2 - testsecondstr.length() / 2);

    m_tiltleLabel->setFocusEnabled(false);
    m_firstSubTiltleLabel->setFocusEnabled(false);
    m_secondSubTiltleLabel->setFocusEnabled(false);

    m_basicenvironmentlist = new NcursesCheckBoxList(this, height() - 15, width() / 2 - 2, begy() + 7,  begx() + 2);
    //m_basicenvironmentlist->setFocus(true);
    m_basicenvironmentlist->setListType(NcursesCheckBoxList::BASICENVIRONMENT);
    m_basicenvironmentlist->setSingleSelect(true);
    m_basicenvironmentlist->setRealSelect(true);

    m_extrachoiceslist = new NcursesCheckBoxList(this, height() - 15, width() / 2 - 2, begy() + 7,  begx() + width() / 2);
    //m_extrachoiceslist->setFocus(true);
    m_extrachoiceslist->setListType(NcursesCheckBoxList::EXTRACHOICES);
    m_extrachoiceslist->setSingleSelect(false);
    m_extrachoiceslist->setRealSelect(false);

    m_selectallextra = new NcursesCheckBox(this, ::QObject::tr("Select All"), 1, width() / 2 - 2, begy() + height() - 7,  begx() + width() / 2  + 1);
}

void InstallComponentFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    printTitle(::QObject::tr("Select Components"), width());

    QString testtiltle     = ::QObject::tr("Select the components according to your needs.");
    QString testfirststr   = ::QObject::tr("Basic Environment");
    QString testsecondstr  = ::QObject::tr("Add-Ons");
    m_tiltleLabel->erase();
    m_firstSubTiltleLabel->erase();
    m_secondSubTiltleLabel->erase();
    if(installer::ReadLocale() == "zh_CN") {
        m_tiltleLabel->resizew(1, testtiltle.length() * 2);
        m_firstSubTiltleLabel->resizew(1, testfirststr.length() * 2);
        m_secondSubTiltleLabel->resizew(1, testsecondstr.length() * 2);
        m_selectallextra->setText("", ::QObject::tr("Select All"), true);
    } else {
        m_tiltleLabel->resizew(1, testtiltle.length());
        m_firstSubTiltleLabel->resizew(1, testfirststr.length());
        m_secondSubTiltleLabel->resizew(1, testsecondstr.length());
        m_selectallextra->setText("", ::QObject::tr("Select All"), false);
    }
    m_tiltleLabel->setText(testtiltle.toUtf8().data());
    m_firstSubTiltleLabel->setText(testfirststr.toUtf8().data());
    m_secondSubTiltleLabel->setText(testsecondstr.toUtf8().data());

    if(m_localeString.compare("") && m_localeString.compare(installer::ReadLocale())) {
        initInfoList();
    } else {
        m_localeString = installer::ReadLocale();
    }

    FrameInterfacePrivate::updateTs();

    layout();
}

void InstallComponentFramePrivate::leftHandle()
{
    if (m_extrachoiceslist->isOnFoucs()) {
        m_basicenvironmentlist->setFocus(true);
        m_extrachoiceslist->setFocus(false);
    }
}

void InstallComponentFramePrivate::rightHandle()
{
    if (m_basicenvironmentlist->isOnFoucs()) {
        m_basicenvironmentlist->setFocus(false);
        m_extrachoiceslist->setFocus(true);
    }
}

void InstallComponentFramePrivate::initConnection()
{
    connect(m_basicenvironmentlist, SIGNAL(signal_KeyTriger(int, int, int)), this, SLOT(slot_KeyTriger(int, int, int)));
    connect(m_extrachoiceslist, SIGNAL(signal_KeyTriger(int, int, int)), this, SLOT(slot_KeyTriger(int, int, int)));
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

    m_serverList = manager->list();

    QVector<QPair<QString, QString>> basicenvironmentinfolist;
    for (auto it = m_serverList.cbegin(); it != m_serverList.cend(); ++it) {
        QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(*it);
        if(tsPair.first.compare("")) {
            basicenvironmentinfolist.push_back(tsPair);
        }
    }

    if (basicenvironmentinfolist.size() > 0) {
        QVector<QPair<QString, QString>> extrachoicesinfolist;
        foreach(QSharedPointer<ComponentInfo> testinfo, m_serverList.first()->extra())
        {
            QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(testinfo->Id);
            extrachoicesinfolist.push_back(tsPair);
        }

        bool testiswchar = false;
        if(installer::ReadLocale() == "zh_CN") {
            testiswchar = true;
        } else {
            testiswchar = false;
        }

        QStringList testitems;
        if (m_serverList.size() > 0) {
            for (int i = 0; i< m_serverList.first()->defaultValue().size(); i++) {
                m_serverList.first()->defaultValue()[i]->Selected = true;
            }
            QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(m_serverList.first());
            testitems.append(tsPair.first);
        }
        m_basicenvironmentlist->setSelectItems(testitems);
        testitems.clear();
        m_extrachoiceslist->setSelectItems(testitems);
        m_basicenvironmentlist->setList(basicenvironmentinfolist, testiswchar);
        m_extrachoiceslist->setList(extrachoicesinfolist, testiswchar);
    }
}

void InstallComponentFramePrivate::writeInfoList()
{
    WriteComponentPackages("");
    WriteComponentUninstallPackages("");

    ComponentInstallManager* manager = ComponentInstallManager::Instance();

    // Write about language
    const QStringList packages =
        ComponentInstallManager::Instance()->loadStructForLanguage(
            installer::ReadLocale());

    if (!packages.isEmpty()) {
        WriteComponentLanguage(packages.join(" "));
    }

    for (auto it = m_serverList.cbegin(); it != m_serverList.cend(); ++it) {
        QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(*it);

        if(!tsPair.first.compare(m_basicenvironmentlist->getCurrentTitle())) {
            const QStringList installPackages = manager->packageListByComponentStruct(*it);
            if (!installPackages.isEmpty()) {
                WriteComponentPackages(installPackages.join(" "));
            }

            bool isMinimalGraphicInstall = true;
            foreach(QSharedPointer<ComponentInfo> testinfo, (*it)->extra())
            {
                if (testinfo->Selected) {
                    isMinimalGraphicInstall = false;
                    break;
                }
            }

            const QStringList uninstallPackages = manager->uninstallPackageListByComponentStruct(*it, isMinimalGraphicInstall);
            if (!uninstallPackages.isEmpty()) {
                WriteComponentUninstallPackages(uninstallPackages.join(" "));
            }

            break;
        }
    }
}


void InstallComponentFramePrivate::slot_KeyTriger(int keycode, int listtype, int index)
{
    switch (keycode) {
    case KEY_UP:
    case KEY_DOWN:
        if(listtype == NcursesCheckBoxList::BASICENVIRONMENT) {
            QVector<QPair<QString, QString>> testinfolist;
            QStringList testselcetitems;
            for (auto it = m_serverList.cbegin(); it != m_serverList.cend(); ++it) {
                QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(*it);

                if(!tsPair.first.compare(m_basicenvironmentlist->getCurrentTitle())) {
                    for (int i = 0; i< (*it)->defaultValue().size(); i++) {
                        (*it)->defaultValue()[i]->Selected = true;
                    }

                    bool testisallselext = true;
                    foreach(QSharedPointer<ComponentInfo> testinfo, (*it)->extra())
                    {
                        QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(testinfo->Id);
                        if(testinfo->Selected){
                            testselcetitems.append(tsPair.first);
                        } else {
                            testisallselext = false;
                        }

                        testinfolist.push_back(tsPair);
                    }
                    m_selectallextra->setSelect(testisallselext);

                } else {
                    for (int i = 0; i< (*it)->defaultValue().size(); i++) {
                        (*it)->defaultValue()[i]->Selected = false;
                    }
                }
            }
            m_extrachoiceslist->setSelectItems(testselcetitems);
            m_extrachoiceslist->setList(testinfolist);
            m_extrachoiceslist->show();
            refresh();
        }
        break;
    case 32:
        if(listtype == NcursesCheckBoxList::EXTRACHOICES) {
            for (auto it = m_serverList.cbegin(); it != m_serverList.cend(); ++it) {
                QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(*it);
                if(!tsPair.first.compare(m_basicenvironmentlist->getCurrentTitle())) {
                    QStringList testselects = m_extrachoiceslist->getSelectItems();
                    bool testisallselext = true;
                    for(int i = 0; i < (*it)->extra().size(); i++) {
                        QPair<QString, QString> tsPair_temp = ComponentInstallManager::Instance()->updateTs((*it)->extra().at(i)->Id);
                        if(testselects.indexOf(tsPair_temp.first) != -1) {
                            (*it)->extra()[i]->Selected = true;
                        } else {
                            (*it)->extra()[i]->Selected = false;
                            testisallselext = false;
                        }
                    }
                    m_selectallextra->setSelect(testisallselext);
                }
            }
        }
        break;
    default:
        break;
    }
}

void InstallComponentFramePrivate::slot_SelectChange(bool select)
{
    for (auto it = m_serverList.cbegin(); it != m_serverList.cend(); ++it) {
        QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(*it);
        if(!tsPair.first.compare(m_basicenvironmentlist->getCurrentTitle())) {
            QStringList testselects = m_extrachoiceslist->getSelectItems();
            for(int i = 0; i < (*it)->extra().size(); i++) {
                QPair<QString, QString> tsPair_temp = ComponentInstallManager::Instance()->updateTs((*it)->extra().at(i)->Id);
                (*it)->extra()[i]->Selected = select;
            }
        }
    }
    m_extrachoiceslist->selectAll(select);
}


void InstallComponentFramePrivate::layout()
{
    m_tiltleLabel->mvwin(begy() + 2,  begx() + (width() / 2) - (m_tiltleLabel->width() / 2));
    m_firstSubTiltleLabel->mvwin(begy() + 5, begx() + 2 + ((width() / 2 - 2) / 2) - m_firstSubTiltleLabel->width() / 2);
    m_secondSubTiltleLabel->mvwin(begy() + 5, begx() + 2 + ((width() / 2 - 2) / 2) - m_secondSubTiltleLabel->width() / 2 + width() / 2);

    m_tiltleLabel->show();
    m_firstSubTiltleLabel->show();
    m_secondSubTiltleLabel->show();

    m_tiltleLabel->refresh();
    m_firstSubTiltleLabel->refresh();
    m_secondSubTiltleLabel->refresh();
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
        m_currState = FRAME_STATE_RUNNING;
    }
    return true;
}

QString ComponentFrame::getFrameName()
{
    return "SelectSoftwareFrame";
}

bool ComponentFrame::handle()
{
    return true;
}


}
