#ifndef COMMAND_H
#define COMMAND_H

#include <QStack>
#include <QObject>
#include <QShortcut>
#include <QString>

// 命令基类
class Command {
public:
    Command() {}
    virtual ~Command() {}
    
    // 执行命令
    virtual void execute() = 0;
    
    // 撤销命令
    virtual void undo() = 0;
    
    // 重做命令 (默认实现是再次执行)
    virtual void redo() { execute(); }
    
    // 获取命令描述（用于记录和显示）
    virtual QString description() const = 0;
};

// 命令管理器类
class CommandManager : public QObject {
    Q_OBJECT
    
public:
    static CommandManager* instance();
    
    // 执行命令
    void executeCommand(Command* command);
    
    // 撤销命令
    bool canUndo() const;
    void undo();
    
    // 重做命令
    bool canRedo() const;
    void redo();
    
    // 清空所有命令历史
    void clearHistory();
    
signals:
    void undoAvailable(bool available);
    void redoAvailable(bool available);
    void commandExecuted();
    void historyChanged();
    
private:
    CommandManager(QObject* parent = nullptr);
    ~CommandManager();
    
    // 单例实例
    static CommandManager* m_instance;
    
    // 命令栈
    QStack<Command*> m_undoStack;
    QStack<Command*> m_redoStack;
    
    // 最大堆栈大小
    static const int MAX_STACK_SIZE = 100;
};

#endif // COMMAND_H 