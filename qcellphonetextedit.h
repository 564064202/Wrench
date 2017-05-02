/* -*- mode: c++ -*- */
#ifndef _QCELLPHONETEXTEDIT_H_
#define _QCELLPHONETEXTEDIT_H_
#include <QTextEdit>
#include <lua.hpp>
#include <QSettings>

class QCellPhoneTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit QCellPhoneTextEdit(QWidget *parent = 0);
    ~QCellPhoneTextEdit();
    QString getMyText();
    void dropEvent(QDropEvent *ev);

signals:
    void controlEnterPressed();
    void emojiShortcutPressed();
    void phoneCallShortcutPressed();
    void imageDropEvent(const QDropEvent& ev);

public slots:
    void on_emojiSelected(const QString& emojiPath);
private:
    QString replaceImagesWithEmoji(const QString& text, const QString& html);
    void keyPressEvent(QKeyEvent *);
    void resizeImages();
    QSettings mSettings;
    lua_State* L;
};

#endif /* _QCELLPHONETEXTEDIT_H_ */
