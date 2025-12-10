#include <bits/stdc++.h>
using namespace std;

// ANSI color codes for pretty output (may need enabling on some terminals)
const string RESET   = "\033[0m";
const string BOLD    = "\033[1m";
const string CYAN    = "\033[96m";
const string GREEN   = "\033[92m";
const string YELLOW  = "\033[93m";
const string MAGENTA = "\033[95m";
const string RED     = "\033[91m";

struct Process {
    int pid;        // process id (1-based)
    int at;         // arrival time
    int bt;         // burst time
    int pr;         // priority (smaller value = higher priority)

    // These will be filled by each algorithm
    int st;         // first start time
    int ct;         // completion time
    int wt;         // waiting time
    int tat;        // turnaround time
    int rt;         // response time
};

struct GanttBlock {
    string label;   // e.g. P1, Idle
    int start;
    int end;
};

struct Averages {
    double wt;
    double tat;
    double rt;
};

void printWelcome() {
    cout << CYAN << BOLD;
    cout << "===============================================\n";
    cout << "   Operating Systems Project - Simulation\n";
    cout << "   CPU Scheduling & Banker's Algorithm\n";
    cout << "===============================================\n";
    cout << RESET;

    cout << MAGENTA << "This project was prepared by:" << RESET << "\n";
    cout << "  - Bayan Mohammed Alotaibi    \n";
    cout << "  - Rawad Eid Alotaibi         \n";
    cout << "  - Layan Alfahmi              \n";
    cout << "  - Dalia Alghamdi             \n";
    cout << "  - Asayel Fayez               \n\n";

    cout << GREEN
         << "Welcome! This program simulates classical CPU scheduling algorithms\n"
         << "and the Banker's Algorithm for deadlock avoidance.\n" << RESET;
    cout << YELLOW
         << "Use the menu to choose which part you want to run, then follow the\n"
         << "instructions on the screen.\n\n" << RESET;
}

void printGantt(const vector<GanttBlock>& g) {
    if (g.empty()) return;

    cout << CYAN << "\nGantt Chart:" << RESET << "\n";

    // Top bar
    cout << " ";
    for (auto &b : g) {
        int width = b.end - b.start;
        if (width < 1) width = 1;
        cout << "+" << string(width, '-');
    } ;
    cout << "+\n";

    // Process labels
    cout << " ";
    for (auto &b : g) {
        int width = b.end - b.start;
        if (width < 1) width = 1;
        int space = width;
        int left = (space - (int)b.label.size()) / 2;
        if (left < 0) left = 0;
        cout << "|" << string(left, ' ') << b.label;
        int remaining = space - left - (int)b.label.size();
        if (remaining < 0) remaining = 0;
        cout << string(remaining, ' ');
    }
    cout << "|\n";

    // Bottom bar
    cout << " ";
    for (auto &b : g) {
        int width = b.end - b.start;
        if (width < 1) width = 1;
        cout << "+" << string(width, '-');
    }
    cout << "+\n";

    // Time markers
    int current = g.front().start;
    cout << " " << current;
    for (auto &b : g) {
        int width = b.end - b.start;
        if (width < 1) width = 1;
        cout << string(width, ' ');
        if (b.end < 10 && current < 10) cout << " "; // small alignment tweak
        cout << b.end;
        current = b.end;
    }
    cout << "\n";
}

Averages computeAverages(vector<Process> &procs) {
    double sumWT = 0, sumTAT = 0, sumRT = 0;
    for (auto &p : procs) {
        p.tat = p.ct - p.at;
        p.wt  = p.tat - p.bt;
        p.rt  = p.st - p.at;
        sumWT  += p.wt;
        sumTAT += p.tat;
        sumRT  += p.rt;
    }
    Averages a;
    int n = (int)procs.size();
    a.wt  = sumWT  / n;
    a.tat = sumTAT / n;
    a.rt  = sumRT  / n;
    return a;
}

void printTable(const string &title, vector<Process> procs, const Averages &avg) {
    // sort by pid for nicer view
    sort(procs.begin(), procs.end(), [](const Process &a, const Process &b){
        return a.pid < b.pid;
    });

    cout << BOLD << "\n=== " << title << " ===\n" << RESET;
    cout << left
         << setw(5)  << "PID"
         << setw(8)  << "AT"
         << setw(8)  << "BT"
         << setw(10) << "PRIORITY"
         << setw(8)  << "ST"
         << setw(8)  << "CT"
         << setw(8)  << "WT"
         << setw(8)  << "TAT"
         << setw(8)  << "RT" << "\n";
    cout << string(71, '-') << "\n";

    for (auto &p : procs) {
        cout << setw(5)  << ("P" + to_string(p.pid))
             << setw(8)  << p.at
             << setw(8)  << p.bt
             << setw(10) << p.pr
             << setw(8)  << p.st
             << setw(8)  << p.ct
             << setw(8)  << p.wt
             << setw(8)  << p.tat
             << setw(8)  << p.rt << "\n";
    }

    cout << GREEN;
    cout << fixed << setprecision(2);
    cout << "Average WT  = " << avg.wt  << "\n";
    cout << "Average TAT = " << avg.tat << "\n";
    cout << "Average RT  = " << avg.rt  << "\n";
    cout << RESET;
}

// FCFS (non-preemptive)
Averages runFCFS(vector<Process> base, vector<GanttBlock> &gantt) {
    // sort by arrival time then pid
    sort(base.begin(), base.end(), [](const Process &a, const Process &b){
        if (a.at == b.at) return a.pid < b.pid;
        return a.at < b.at;
    });

    gantt.clear();
    int time = 0;

    for (auto &p : base) {
        if (time < p.at) {
            // CPU idle
            gantt.push_back({"Idle", time, p.at});
            time = p.at;
        }
        p.st = time;
        time += p.bt;
        p.ct = time;
        gantt.push_back({"P" + to_string(p.pid), p.st, p.ct});
    }

    Averages avg = computeAverages(base);
    printGantt(gantt);
    printTable("FCFS", base, avg);
    return avg;
}

// SJF Non-preemptive
Averages runSJF(vector<Process> base, vector<GanttBlock> &gantt) {
    int n = (int)base.size();
    vector<bool> done(n, false);

    gantt.clear();
    int time = 0, completed = 0;

    while (completed < n) {
        int idx = -1;
        int bestBT = INT_MAX;

        for (int i = 0; i < n; ++i) {
            if (!done[i] && base[i].at <= time) {
                if (base[i].bt < bestBT ||
                   (base[i].bt == bestBT && (idx == -1 || base[i].pid < base[idx].pid))) {
                    bestBT = base[i].bt;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            int nextArrival = INT_MAX;
            for (int i = 0; i < n; ++i) {
                if (!done[i]) nextArrival = min(nextArrival, base[i].at);
            }
            if (nextArrival == INT_MAX) break; // shouldn't happen
            gantt.push_back({"Idle", time, nextArrival});
            time = nextArrival;
            continue;
        }

        base[idx].st = time;
        time += base[idx].bt;
        base[idx].ct = time;
        gantt.push_back({"P" + to_string(base[idx].pid), base[idx].st, base[idx].ct});
        done[idx] = true;
        completed++;
    }

    Averages avg = computeAverages(base);
    printGantt(gantt);
    printTable("SJF (Non-preemptive)", base, avg);
    return avg;
}

// Priority Scheduling (Non-preemptive, smaller priority value = higher priority)
Averages runPriority(vector<Process> base, vector<GanttBlock> &gantt) {
    int n = (int)base.size();
    vector<bool> done(n, false);
    gantt.clear();
    int time = 0, completed = 0;

    while (completed < n) {
        int idx = -1;
        int bestPr = INT_MAX;

        for (int i = 0; i < n; ++i) {
            if (!done[i] && base[i].at <= time) {
                if (base[i].pr < bestPr ||
                   (base[i].pr == bestPr && (idx == -1 || base[i].pid < base[idx].pid))) {
                    bestPr = base[i].pr;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            int nextArrival = INT_MAX;
            for (int i = 0; i < n; ++i) {
                if (!done[i]) nextArrival = min(nextArrival, base[i].at);
            }
            if (nextArrival == INT_MAX) break;
            gantt.push_back({"Idle", time, nextArrival});
            time = nextArrival;
            continue;
        }

        base[idx].st = time;
        time += base[idx].bt;
        base[idx].ct = time;
        gantt.push_back({"P" + to_string(base[idx].pid), base[idx].st, base[idx].ct});
        done[idx] = true;
        completed++;
    }

    Averages avg = computeAverages(base);
    printGantt(gantt);
    printTable("Priority (Non-preemptive)", base, avg);
    return avg;
}

// Round Robin (preemptive)
Averages runRR(vector<Process> base, int quantum, vector<GanttBlock> &gantt) {
    int n = (int)base.size();
    gantt.clear();

    // sort by arrival time, then pid
    sort(base.begin(), base.end(), [](const Process &a, const Process &b){
        if (a.at == b.at) return a.pid < b.pid;
        return a.at < b.at;
    });

    vector<int> remBT(n);
    for (int i = 0; i < n; ++i) {
        remBT[i] = base[i].bt;
        base[i].st = -1;
    }

    queue<int> q;
    int time = 0;
    int completed = 0;
    int next = 0; // next process to arrive (index in sorted base)

    auto addArrivals = [&](int currentTime) {
        while (next < n && base[next].at <= currentTime) {
            q.push(next);
            next++;
        }
    };

    // if no process has arrived yet, jump to first arrival
    if (next < n && time < base[next].at) {
        gantt.push_back({"Idle", time, base[next].at});
        time = base[next].at;
    }
    addArrivals(time);

    while (completed < n) {
        if (q.empty()) {
            if (next < n) {
                // CPU idle until next arrival
                gantt.push_back({"Idle", time, base[next].at});
                time = base[next].at;
                addArrivals(time);
                continue;
            } else {
                break;
            }
        }

        int idx = q.front();
        q.pop();

        if (base[idx].st == -1) {
            base[idx].st = time; // first time it gets CPU
        }

        int execTime = min(quantum, remBT[idx]);
        int start = time;
        time += execTime;
        remBT[idx] -= execTime;
        gantt.push_back({"P" + to_string(base[idx].pid), start, time});

        addArrivals(time);

        if (remBT[idx] == 0) {
            base[idx].ct = time;
            completed++;
        } else {
            q.push(idx);
        }
    }

    Averages avg = computeAverages(base);
    printGantt(gantt);
    printTable("Round Robin (q=" + to_string(quantum) + ")", base, avg);
    return avg;
}

void compareAlgorithms(const Averages &fcfs, const Averages &sjf,
                       const Averages &prio, const Averages &rr) {
    cout << BOLD << "\n=== Comparison between Algorithms (Bonus) ===\n" << RESET;

    vector<string> names = {"FCFS", "SJF", "Priority", "RR"};
    vector<Averages> avgs = {fcfs, sjf, prio, rr};

    double minWT = 1e9, minTAT = 1e9, minRT = 1e9;
    for (auto &a : avgs) {
        minWT  = min(minWT,  a.wt);
        minTAT = min(minTAT, a.tat);
        minRT  = min(minRT,  a.rt);
    }

    cout << left
         << setw(12) << "Algorithm"
         << setw(15) << "Avg WT"
         << setw(15) << "Avg TAT"
         << setw(15) << "Avg RT" << "\n";
    cout << string(57, '-') << "\n";

    cout << fixed << setprecision(2);
    for (size_t i = 0; i < names.size(); ++i) {
        cout << setw(12) << names[i];

        auto printVal = [&](double val, double best) {
            if (fabs(val - best) < 1e-6) {
                cout << GREEN << setw(15) << val << RESET;
            } else {
                cout << setw(15) << val;
            }
        };

        printVal(avgs[i].wt,  minWT);
        printVal(avgs[i].tat, minTAT);
        printVal(avgs[i].rt,  minRT);
        cout << "\n";
    }

    cout << GREEN
         << "\nGreen values indicate the best (smallest) average for that metric.\n"
         << RESET;
}

void cpuSchedulingPart() {
    int n;
    cout << YELLOW << "\n===== Part 1: CPU Scheduling Simulation =====\n" << RESET;
    cout << "Enter number of processes: ";
    cin >> n;

    vector<Process> procs(n);
    cout << CYAN
         << "\nEnter process data (ArrivalTime BurstTime Priority) for each process:\n"
         << RESET;
    for (int i = 0; i < n; ++i) {
        procs[i].pid = i + 1;
        cout << "P" << procs[i].pid << " (AT BT PR): ";
        cin >> procs[i].at >> procs[i].bt >> procs[i].pr;
    }

    int quantum;
    cout << "\nEnter time quantum for Round Robin: ";
    cin >> quantum;

    vector<GanttBlock> gFCFS, gSJF, gPR, gRR;

    Averages aFCFS = runFCFS(procs, gFCFS);
    Averages aSJF  = runSJF(procs,  gSJF);
    Averages aPR   = runPriority(procs, gPR);
    Averages aRR   = runRR(procs, quantum, gRR);

    compareAlgorithms(aFCFS, aSJF, aPR, aRR);
}

void bankersPart() {
    cout << YELLOW << "\n===== Part 2: Banker's Algorithm Simulation =====\n" << RESET;

    int n, m;
    cout << "Enter number of processes: ";
    cin >> n;
    cout << "Enter number of resource types: ";
    cin >> m;

    vector<int> total(m);
    cout << CYAN
         << "\nEnter total instances for each resource type R1..Rm:\n"
         << RESET;
    for (int j = 0; j < m; ++j) {
        cout << "Total instances of R" << j + 1 << ": ";
        cin >> total[j];
    }

    vector<vector<int>> maxm(n, vector<int>(m));
    vector<vector<int>> alloc(n, vector<int>(m));

    cout << CYAN << "\nEnter MAX matrix (maximum demand of each process):\n" << RESET;
    for (int i = 0; i < n; ++i) {
        cout << "Max for P" << i << " (" << m << " values): ";
        for (int j = 0; j < m; ++j) cin >> maxm[i][j];
    }

    cout << CYAN << "\nEnter ALLOCATION matrix (current allocation for each process):\n"
         << RESET;
    for (int i = 0; i < n; ++i) {
        cout << "Alloc for P" << i << " (" << m << " values): ";
        for (int j = 0; j < m; ++j) cin >> alloc[i][j];
    }

    // Compute Available automatically: total - sum(allocation)
    vector<int> available(m);
    for (int j = 0; j < m; ++j) {
        int sumAlloc = 0;
        for (int i = 0; i < n; ++i) sumAlloc += alloc[i][j];
        available[j] = total[j] - sumAlloc;
    }

    vector<vector<int>> need(n, vector<int>(m));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            need[i][j] = maxm[i][j] - alloc[i][j];
        }
    }

    cout << MAGENTA << "\nComputed AVAILABLE vector (Total - Allocation):\n" << RESET;
    for (int j = 0; j < m; ++j) cout << "R" << j + 1 << "=" << available[j] << " ";
    cout << "\n";

    cout << CYAN << "\nNEED matrix (Max - Allocation):\n" << RESET;
    for (int i = 0; i < n; ++i) {
        cout << "P" << i << ": ";
        for (int j = 0; j < m; ++j) cout << setw(3) << need[i][j] << " ";
        cout << "\n";
    }

    // Safety algorithm
    vector<int> work = available;
    vector<bool> finish(n, false);
    vector<int> safeSeq;

    while (true) {
        bool found = false;
        for (int i = 0; i < n; ++i) {
            if (!finish[i]) {
                bool canFinish = true;
                for (int j = 0; j < m; ++j) {
                    if (need[i][j] > work[j]) {
                        canFinish = false;
                        break;
                    }
                }
                if (canFinish) {
                    for (int j = 0; j < m; ++j) work[j] += alloc[i][j];
                    finish[i] = true;
                    safeSeq.push_back(i);
                    found = true;
                }
            }
        }
        if (!found) break;
    }

    bool safe = true;
    for (int i = 0; i < n; ++i) if (!finish[i]) safe = false;

    if (safe) {
        cout << GREEN << "\nThe system is in a SAFE state.\nSafe sequence: <";
        for (size_t i = 0; i < safeSeq.size(); ++i) {
            cout << "P" << safeSeq[i];
            if (i + 1 != safeSeq.size()) cout << " -> ";
        }
        cout << ">" << RESET << "\n";
    } else {
        cout << RED
             << "\nThe system is in an UNSAFE state (no safe sequence exists)."
             << RESET << "\n";
    }
}

int main() {

    printWelcome();

    while (true) {
        cout << BOLD << "\nMain Menu" << RESET << "\n";
        cout << " 1) CPU Scheduling Simulation\n";
        cout << " 2) Banker's Algorithm Simulation\n";
        cout << " 0) Exit\n";
        cout << "Enter your choice: ";

        int choice;
        if (!(cin >> choice)) break;

        if (choice == 1) {
            cpuSchedulingPart();
        } else if (choice == 2) {
            bankersPart();
        } else if (choice == 0) {
            cout << GREEN << "\nThank you for using the simulator. Goodbye!\n" << RESET;
            break;
        } else {
            cout << RED << "Invalid choice. Please try again.\n" << RESET;
        }
    }

    return 0;
}
