#include "install_sccuess_frame.h"


namespace installer {


class InstallSuccessFramePrivate : public FrameInterfacePrivate
{
public:
    InstallSuccessFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
    {

    }


    // FrameInterfacePrivate interface
public:
    void initUI();
};

void InstallSuccessFramePrivate::initUI()
{
    InstallSuccessFramePrivate::initUI();
    printTitle(::QObject::tr("language select"), width());
}





LanguageFrame::LanguageFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new LanguageFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
}

LanguageFrame::~LanguageFrame()
{

}


bool LanguageFrame::init()
{
    m_private->initUI();
    m_private->refresh();
    return true;
}

QString LanguageFrame::getFrameName()
{
    return "LanguageFrame";
}

bool LanguageFrame::handle()
{
    {
        //do something
    }
    getKey();
    return true;
}

}
