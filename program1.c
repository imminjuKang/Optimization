#include <stdio.h>
#define TIME_QUANTUM0 2
#define TIME_QUANTUM1 4
#define MAX_LENGTH 1000

typedef struct Process
{
    int pid;
    int arrival_time;
    int burst_time;
    int remain_time;
    int finish_time;
    int current_q;

} Process;

typedef struct Queue
{
    int front;
    int rear;
    int size;
    Process *process_info[MAX_LENGTH];

} Queue;

void init_queue(Queue *q)
{
    q -> front = q -> rear = q -> size = 0;
}

void enqueue(Queue *q, Process *p)
{
    q->process_info[q->rear] = p;
    q->rear++;
    q->size++;
}

Process *dequeue(Queue *q)
{
    Process *p = q->process_info[q->front];
    q->front++;
    q->size--;
    return p;
}

int isEmpty(Queue *q)
{
    if (q->front == q->rear)
        return 1;
    else
        return 0;
}

Process *spn(Queue *q)
{
    if (q->size == 0)
        return NULL;

    int min = q->front;
    for (int i = q->front + 1; i < q->rear; i++)
    {
        if (q->process_info[i]->remain_time < q->process_info[min]->remain_time)
        {
            min = i;
        }
    }

    Process *shortest = q->process_info[min];

    for (int i = min; i < q->rear - 1; i++)
    {
        q->process_info[i] = q->process_info[i + 1];
    }
    
    q->rear--;
    q->size--;

    return shortest;
}

int read_file(const char *file, Process (*process)[MAX_LENGTH], int *num_process_ptr)
{
    FILE *fp = fopen(file, "r");
    if (fp == NULL)
    {
        printf("Error: Cannot open file");
        return 0;
    }

    int num_process = 0;
    if (fscanf(fp, "Number of Processes: %d\n", &num_process) != 1) {
        fprintf(stderr, "Error: Invalid input format.\n");
        fclose(fp);
        return 0;
    }

    if (num_process > MAX_LENGTH) {
        fprintf(stderr, "Error: Too many processes. Max is %d\n", MAX_LENGTH);
        fclose(fp);
        return 0;
    }

    for (int i = 0; i < num_process; i++) {
        if (fscanf(fp, "%d, %d, %d\n", &(*process)[i].pid, &(*process)[i].arrival_time, &(*process)[i].burst_time) != 3) {
            fprintf(stderr, "Error: Invalid process data on line %d\n", i + 2);
            fclose(fp);
            return 0;
        }
        (*process)[i].remain_time = (*process)[i].burst_time;
    }

    fclose(fp);
    *num_process_ptr = num_process;
    return 1;

}

void enqueue_arrival(Process *process, int num, int time, Queue *q0)
{
    for (int i = 0; i < num; i++)
    {
        if (process[i].arrival_time == time)
        {
            enqueue(q0, &process[i]);
        }
    }
}

Process* next_p(Queue *queue, int level)
{
    Process *p = dequeue(queue);
    if (p)
    {
        p -> current_q = level;
    }
    return p;
}

Process* scheduling(Queue *q0, Queue *q1, Queue *q2)
{
    if (!isEmpty(q0))
    {
        return next_p(q0, 0);
    }
    if (!isEmpty(q1))
    {
        return next_p(q1, 1);
    }
    if (!isEmpty(q2))
    {
        return next_p(q2, 2);
    }
    return NULL;
}

int expiration(Process **process, int *time_quantum, Queue *q1, Queue *q2)
{
    if ((*process)->current_q == 0 && *time_quantum == TIME_QUANTUM0)
    {
        enqueue(q1, *process);
        *process = NULL;
        *time_quantum = 0;
        return 1;
    }
    else if ((*process)->current_q == 1 && *time_quantum == TIME_QUANTUM1)
    {
        enqueue(q2, *process);
        *process = NULL;
        *time_quantum = 0;
        return 1;
    }
    return 0;
}

void execution(Process **process, int *time_quantum, int time, int *chart, Queue *q1, Queue *q2)
{
    chart[time] = (*process)->pid;
    (*process)->remain_time--;
    (*time_quantum)++;

    if ((*process)->remain_time == 0)
    {
        (*process)->finish_time = time + 1;
        *process = NULL;
        *time_quantum = 0;
        return ;
    }

    if (expiration(process, time_quantum, q1, q2)) return;
}

void gantt_char(int *gantt_chart, int clock_time)
{
    printf("Gantt Chart\n");

    int prev = -1;
    for (int i = 0; i < clock_time; i++)
    {
        if (gantt_chart[i] != prev)
        {
            printf("|P%-2d", gantt_chart[i]);
            prev = gantt_chart[i];
        }
    }
    printf("|\n");

    prev = -1;
    for (int i = 0; i < clock_time; i++)
    {
        if (gantt_chart[i] != prev)
        {
            printf("%-4d", i);
            prev = gantt_chart[i];
        }
    }

    printf("%d\n", clock_time);
}

void print_chart(Process *process, int num_process)
{
    int turnaround_time = 0;
    int waiting_time = 0;
    double avg_tt = 0;
    double avg_wt = 0;


    printf("\nPID   TT     WT\n");
    printf("==================\n");
    for (int i = 0; i < num_process; i++)
    {
        turnaround_time = process[i].finish_time - process[i].arrival_time;
        waiting_time = turnaround_time - process[i].burst_time;
        avg_tt += turnaround_time;
        avg_wt += waiting_time;
        printf("P%d %5d %6d\n", process[i].pid, turnaround_time, waiting_time);
    }

    printf("\nAvergae TT: %.1f\n", avg_tt / num_process);
    printf("Average WT: %.1f", avg_wt / num_process);
}

int main()
{

    int num_process = 0;
    Process process[MAX_LENGTH];

    if (!read_file("input.txt", &process, &num_process)) 
    {
    return 1; 
    }

    Queue q0, q1, q2;
    init_queue(&q0);
    init_queue(&q1);
    init_queue(&q2);

    int gantt_chart[MAX_LENGTH] = {0};
    int clock_time = 0;

    Process *running_p = NULL;
    int time_quantum = 0;

    while (1)
    {
        enqueue_arrival(process, num_process, clock_time, &q0);

        if (running_p != NULL)
        {
            execution(&running_p, &time_quantum, clock_time, gantt_chart, &q1, &q2);
        }

        if (running_p == NULL)
        {
            running_p = scheduling(&q0, &q1, &q2);
            if (running_p == NULL) break;
        }

        clock_time++;
    }

    gantt_char(gantt_chart, clock_time);
    print_chart(process, num_process);

}
