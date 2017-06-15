#ifndef FILTERINGMODEL_H
#define FILTERINGMODEL_H

#include <QAbstractTableModel>
#include <QAbstractListModel>
#include <QMap>
#include <QPixmap>
#include <QSharedPointer>
#include <lua.hpp>
#include <QSettings>
#include "selectoutput.h"
#include "vcard.h"

class FilteringModel : public QAbstractListModel
{
    Q_OBJECT
 public:
    explicit FilteringModel(QObject *parent = 0);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual void setFilter(QString filter);
    virtual void on_indexSelected(int row) {};

 protected:
    QList<SelectedItem> mSelectedItems;
    lua_State *L;
    virtual void filterSelectedItems(const QStringList&) = 0;
    virtual QString getHistoryName() = 0;
    QMap<QString, SelectedItem> mSelectedItemsRevMap;

    QSettings mSettings;
    virtual void initHistory();
    virtual QString getNthHistoryVarName(int n);
    int mMaxHistEntries;

 private:
    QString mInitFilter;
    QString mFilter;
    QList<QString> mHistoryList;
    void sortEntriesWithHistory(int oldRows = 0);

 public:
    void setInitFilter(QString initFilter) { mInitFilter = initFilter; };
    virtual void maybeAddTextIntoHistory(const QString& text) {};
    QString getSelectedText(int i);
    QString getSelectedDisplayText(int i);
    virtual QMap<QString, QString> getSelectedRawData(int i);
    void updateHistory(int i);
    void updateHistory(QString key);
signals:
    void iconsUpdated();
public slots:

};

#endif // FILTERINGMODEL_H
