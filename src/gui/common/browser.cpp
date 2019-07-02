#include "browser.h"

#include <QtWidgets/QtWidgets>
#include <QtWidgets/QFileSystemModel>


// ----------------------------------------------------------------------------

class BrowserModel : public QFileSystemModel
{
  public:
    BrowserModel(QObject *parent = nullptr) : QFileSystemModel(parent)
    {
        setFilter(QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot);
    }

  public:
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return 1;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::ToolTipRole)
            role = Qt::DisplayRole;

        return QFileSystemModel::data(index, role);
    }
};

// ----------------------------------------------------------------------------

BrowserWidget::BrowserWidget(QWidget *parent)
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(4);

    m_browser = new BrowserListWidget();
    vLayout->addWidget(m_browser);
    m_leSearch = new QLineEdit();
    m_leSearch->setPlaceholderText("Search...");
    m_leSearch->setClearButtonEnabled(true);
    vLayout->addWidget(m_leSearch);

    QObject::connect(m_leSearch, &QLineEdit::textChanged, m_browser, &BrowserListWidget::filterList);
    QObject::connect(m_browser, &BrowserListWidget::doubleClicked, this, &BrowserWidget::updateSelection);
}

void BrowserWidget::setRootPath(const QString &path)
{
    m_browser->setRootPath(path);
}

void BrowserWidget::setExtFilter(const QStringList &filter)
{
    m_browser->setExtFilter(filter);
}

void BrowserWidget::updateSelection(const QModelIndex &index)
{
    QString path = index.data(QFileSystemModel::FilePathRole).toString();
    EmitEvent<Select>(path);
}

// ----------------------------------------------------------------------------

BrowserListWidget::BrowserListWidget(QWidget *parent)
:   QTreeView(parent)
{
    m_fileSystemModel = new BrowserModel(this);
    m_sortFilterModel = new QSortFilterProxyModel(this);

    m_sortFilterModel->setFilterRole(QFileSystemModel::FilePathRole);
    m_sortFilterModel->setRecursiveFilteringEnabled(true);
    m_sortFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_sortFilterModel->setSourceModel(m_fileSystemModel);

    setModel(m_sortFilterModel);

    setSelectionMode(QAbstractItemView::SingleSelection);
    header()->setVisible(false);
    // This is required to enable horizontal scrolling
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void BrowserListWidget::setRootPath(const QString &path)
{
    m_rootPath = path;
    QModelIndex idx = m_sortFilterModel->mapFromSource( m_fileSystemModel->setRootPath(path) );
    setRootIndex(idx);
}

void BrowserListWidget::setExtFilter(const QStringList &filter)
{
    m_extFilter = filter;
    m_fileSystemModel->setNameFilters(m_extFilter);
    m_fileSystemModel->setNameFilterDisables(false);
}

void BrowserListWidget::filterList(const QString &filter)
{
    expandAll();
    m_sortFilterModel->setFilterFixedString(filter);

    if (filter.isEmpty() || !rootIndex().isValid()) {
        setRootPath(m_rootPath);
        collapseAll();
    }
}
