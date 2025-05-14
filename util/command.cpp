#include "command.h"

// 初始化静态实例
CommandManager* CommandManager::m_instance = nullptr;

// 获取单例实例
CommandManager* CommandManager::instance() {
    if (!m_instance) {
        m_instance = new CommandManager();
    }
    return m_instance;
}

// 构造函数
CommandManager::CommandManager(QObject* parent) : QObject(parent) {
}

// 析构函数
CommandManager::~CommandManager() {
    clearHistory();
}

// 执行命令
void CommandManager::executeCommand(Command* command) {
    if (!command) return;
    
    // 执行命令
    command->execute();
    
    // 添加到撤销栈
    m_undoStack.push(command);
    
    // 清空重做栈（因为有新的操作）
    while (!m_redoStack.isEmpty()) {
        Command* cmd = m_redoStack.pop();
        delete cmd;
    }
    
    // 如果撤销栈超过最大限制，移除最早的命令
    if (m_undoStack.size() > MAX_STACK_SIZE) {
        Command* oldestCmd = m_undoStack.takeAt(0);
        delete oldestCmd;
    }
    
    // 发出信号
    emit undoAvailable(true);
    emit redoAvailable(false);
    emit commandExecuted();
    emit historyChanged();
}

// 判断是否可以撤销
bool CommandManager::canUndo() const {
    return !m_undoStack.isEmpty();
}

// 撤销操作
void CommandManager::undo() {
    if (canUndo()) {
        // 从撤销栈取出最近的命令
        Command* command = m_undoStack.pop();
        
        // 执行撤销
        command->undo();
        
        // 添加到重做栈
        m_redoStack.push(command);
        
        // 发出信号
        emit undoAvailable(canUndo());
        emit redoAvailable(true);
        emit historyChanged();
    }
}

// 判断是否可以重做
bool CommandManager::canRedo() const {
    return !m_redoStack.isEmpty();
}

// 重做操作
void CommandManager::redo() {
    if (canRedo()) {
        // 从重做栈取出最近的命令
        Command* command = m_redoStack.pop();
        
        // 执行重做
        command->redo();
        
        // 添加回撤销栈
        m_undoStack.push(command);
        
        // 发出信号
        emit undoAvailable(true);
        emit redoAvailable(canRedo());
        emit historyChanged();
    }
}

// 清空历史
void CommandManager::clearHistory() {
    // 清空撤销栈
    while (!m_undoStack.isEmpty()) {
        Command* cmd = m_undoStack.pop();
        delete cmd;
    }
    
    // 清空重做栈
    while (!m_redoStack.isEmpty()) {
        Command* cmd = m_redoStack.pop();
        delete cmd;
    }
    
    // 发出信号
    emit undoAvailable(false);
    emit redoAvailable(false);
    emit historyChanged();
} 