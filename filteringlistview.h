#ifndef FILTERINGLISTVIEW_H
#define FILTERINGLISTVIEW_H

#include <QListView>

class FilteringListView : public QListView
{
    Q_OBJECT
public:
    explicit FilteringListView(QWidget *parent = 0);

signals:
    void selectedCurrentEntry(const QModelIndex&);
    void selectedCurrentText(const QString&);
    void selectedCurrentEntryNoHistory(const QModelIndex&);
    void sendCurrentEntryToEdit(const QString&);
    void selectRawData(const QMap<QString, QString>&);

public slots:
    void nextEntry();
    void getCurrentEntryForEdit();
    void prevEntry();
    void firstEntry();
    void lastEntry();
    void nextPageOfEntries();
    void prevPageOfEntries();
    void selectedCurrentEntryWithText(const QString& input = "");
    void selectAllEntries();
    void shiftSelectCurrentEntry();

protected slots:
    void dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles = QVector<int> ());

private:
    void changeSelection(int how);
};

#endif // FILTERINGLISTVIEW_H
