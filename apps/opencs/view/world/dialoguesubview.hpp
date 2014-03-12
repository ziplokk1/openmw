#ifndef CSV_WORLD_DIALOGUESUBVIEW_H
#define CSV_WORLD_DIALOGUESUBVIEW_H

#include <map>
#include <memory>

#include <QAbstractItemDelegate>
#include <QScrollArea>

#include "../doc/subview.hpp"
#include "../../model/world/columnbase.hpp"

class QDataWidgetMapper;
class QSize;
class QEvent;
class QLabel;
class QVBoxLayout;

namespace CSMWorld
{
    class IdTable;
}

namespace CSMDoc
{
    class Document;
}

namespace CSVWorld
{
    class CommandDelegate;

    class NotEditableSubDelegate : public QAbstractItemDelegate
    {
        const CSMWorld::IdTable* mTable;
    public:
        NotEditableSubDelegate(const CSMWorld::IdTable* table, QObject * parent = 0);

        virtual void setEditorData (QLabel* editor, const QModelIndex& index) const;

        virtual void setModelData (QWidget* editor, QAbstractItemModel* model, const QModelIndex& index, CSMWorld::ColumnBase::Display display) const;

        virtual void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        ///< does nothing

        virtual QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const;
        ///< does nothing

        virtual QWidget *createEditor (QWidget *parent,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index,
                                CSMWorld::ColumnBase::Display display = CSMWorld::ColumnBase::Display_None) const;
    };

    //this can't be nested into the DialogueDelegateDispatcher, because it needs to emit signals
    class DialogueDelegateDispatcherProxy : public QObject
    {
        Q_OBJECT
        class refWrapper
        {
        public:
            refWrapper(const QModelIndex& index);

            const QModelIndex& mIndex;
        };

        QWidget* mEditor;

        CSMWorld::ColumnBase::Display mDisplay;

        std::auto_ptr<refWrapper> mIndexWrapper;

    public:
        DialogueDelegateDispatcherProxy(QWidget* editor, CSMWorld::ColumnBase::Display display);
        QWidget* getEditor() const;

    public slots:
        void editorDataCommited();
        void setIndex(const QModelIndex& index);
        void tableMimeDataDropped(const std::vector<CSMWorld::UniversalId>& data);

    signals:
        void editorDataCommited(QWidget* editor, const QModelIndex& index, CSMWorld::ColumnBase::Display display);
    };

    class DialogueDelegateDispatcher : public QAbstractItemDelegate
    {
        Q_OBJECT
        std::map<int, CommandDelegate*> mDelegates;

        QObject* mParent;

        CSMWorld::IdTable* mTable; //nor sure if it is needed TODO

        QUndoStack& mUndoStack;

        NotEditableSubDelegate mNotEditableDelegate;

        std::vector<DialogueDelegateDispatcherProxy*> mProxys; //once we move to the C++11 we should use unique_ptr

    public:
        DialogueDelegateDispatcher(QObject* parent, CSMWorld::IdTable* table, QUndoStack& undoStack);

        ~DialogueDelegateDispatcher();

        CSVWorld::CommandDelegate* makeDelegate(CSMWorld::ColumnBase::Display display);

        QWidget* makeEditor(CSMWorld::ColumnBase::Display display, const QModelIndex& index);
        ///< will return null if delegate is not present, parent of the widget is same as for dispatcher itself

        virtual void setEditorData (QWidget* editor, const QModelIndex& index) const;

        virtual void setModelData (QWidget* editor, QAbstractItemModel* model, const QModelIndex& index, CSMWorld::ColumnBase::Display display) const;

        virtual void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        ///< does nothing

        virtual QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const;
        ///< does nothing

    private slots:
        void editorDataCommited(QWidget* editor, const QModelIndex& index, CSMWorld::ColumnBase::Display display);

    };

    class EditWidget : public QScrollArea
    {
            QDataWidgetMapper *mWidgetMapper;
            DialogueDelegateDispatcher mDispatcher;
            QWidget* mMainWidget;
            CSMWorld::IdTable* mTable;
            QUndoStack& mUndoStack;

        public:

            EditWidget (QWidget *parent, int row, CSMWorld::IdTable* table, QUndoStack& undoStack, bool createAndDelete = false);

            void remake(int row);
    };

    class DialogueSubView : public CSVDoc::SubView
    {
        Q_OBJECT

        EditWidget* mEditWidget;
        QVBoxLayout* mMainLayout;
        CSMWorld::IdTable* mTable;
        QUndoStack& mUndoStack;
        int mRow;
        bool mLocked;

        public:

            DialogueSubView (const CSMWorld::UniversalId& id, CSMDoc::Document& document, bool createAndDelete = false);

            virtual void setEditLock (bool locked);

        private slots:

            void nextId();

            void prevId();

            void dataChanged();
            ///\brief we need to care for deleting currently edited record
    };
}

#endif