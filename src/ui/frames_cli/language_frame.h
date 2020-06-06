#ifndef CLI_INSTALL_LANGUAGE_FRAME_H
#define CLI_INSTALL_LANGUAGE_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"
#include "service/system_language.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_label.h"
namespace installer {
class LanguageFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    LanguageFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
          m_index(0),
          m_isshow(false)

    {
        initUI();
        initConnection();
    }

public:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    bool canBack() override{
        return false;
    }

    void update();

signals:
    void languageChange();

public:
    void readConf();
    void writeConf();

private:
    int m_index;
    LanguageList m_languageList;
    NcursesListView* m_languageView = nullptr;
    NcursesLabel* m_instructions = nullptr;
    //NcursesLabel* m_titleLabel = nullptr;
    bool m_isshow;

};

class LanguageFrame : public FrameInterface
{
    Q_OBJECT
public:
    LanguageFrame(FrameInterface* parent);
    virtual ~LanguageFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

    void update();

signals:
    void languageChanged();

protected:
    bool handle() override;
private:
    Q_DECLARE_PRIVATE_D(m_private, LanguageFrame)
};


}


#endif
