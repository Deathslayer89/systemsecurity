#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

enum class ProcessState { READY, RUNNING, BLOCKED };

class Process {
private:
    int pid;
    int ppid;
    ProcessState state;
    std::vector<uint8_t> addressSpace;
    std::vector<std::shared_ptr<Process>> children;

public:
    Process(int pid, int ppid = 0) : 
        pid(pid), ppid(ppid), state(ProcessState::READY) {
        addressSpace.resize(1024, 0); // Simple 1KB address space
    }

    int getPid() const { return pid; }
    int getPPid() const { return ppid; }
    ProcessState getState() const { return state; }
    void setState(ProcessState newState) { state = newState; }
    
    std::shared_ptr<Process> fork() {
        auto child = std::make_shared<Process>(getNextPid(), pid);
        child->addressSpace = addressSpace; // Copy address space
        children.push_back(child);
        return child;
    }

    bool exec(const std::string& program) {
        // Simulate loading new program
        addressSpace.clear();
        addressSpace.resize(1024, 0);
        // Simulate program loading
        std::cout << "Executing program: " << program << " in process " << pid << std::endl;
        return true;
    }

private:
    static int getNextPid() {
        static int nextPid = 1;
        return nextPid++;
    }
};

class ProcessManager {
private:
    std::unordered_map<int, std::shared_ptr<Process>> processes;
    std::shared_ptr<Process> currentProcess;

public:
    ProcessManager() {
        // Create init process
        auto init = std::make_shared<Process>(0);
        processes[0] = init;
        currentProcess = init;
    }

    std::shared_ptr<Process> fork() {
        if (!currentProcess) return nullptr;
        auto child = currentProcess->fork();
        processes[child->getPid()] = child;
        return child;
    }

    bool exec(const std::string& program) {
        if (!currentProcess) return false;
        return currentProcess->exec(program);
    }

    bool terminateProcess(int pid) {
        if (processes.find(pid) == processes.end()) return false;
        processes.erase(pid);
        return true;
    }

    void switchProcess(int pid) {
        if (processes.find(pid) != processes.end()) {
            if (currentProcess) 
                currentProcess->setState(ProcessState::READY);
            currentProcess = processes[pid];
            currentProcess->setState(ProcessState::RUNNING);
        }
    }
};

// Example usage
int main() {
    ProcessManager pm;
    
    // Create child process
    auto child = pm.fork();
    std::cout << "Created child process: " << child->getPid() << std::endl;
    
    // Execute new program in child
    pm.switchProcess(child->getPid());
    pm.exec("new_program.exe");
    
    // Terminate child
    pm.terminateProcess(child->getPid());
    
    return 0;
}