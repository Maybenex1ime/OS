//Where defined struct task_struct
#include <linux/sched.h>
//where defined initialization and exit
#include <linux/init.h>
//where diefined basical functions and macros
#include <linux/kernel.h>
//where definded function, fieds about kernal module
#include <linux/module.h>
#include <linux/proc_fs.h>
#define WRITEBUF 1024
#define READBUF 5120
struct proc_dir_entry *proc_dir = NULL;
struct proc_dir_entry *proc_file = NULL;
char write_buf[WRITEBUF];
char read_buf[READBUF];

// add macros handle
MODULE_DESCRIPTION("procfs:task_struct, cpu_itimer");
MODULE_VERSION("1");
MODULE_LICENSE("GPL");
LIST_HEAD(pgd_list);
DEFINE_SPINLOCK(pgd_lock);

int pid = 1;
long long length = 0;

int count_proccess(struct task_struct * zero_T){
    struct list_head *pos;
    int count = 0;
    list_for_each(pos,&zero_T->tasks)
    {
        count++;
    }
    return count;
}
void get_state(long state, char ** result){
    if(state==0){
        *result = "RUNNING";
    }else if(state==1){
        *result = "INTERRUPTIBLE SLEEPING";
    }else if(state==2){
        *result = "UNITERRUPTIBLE SLEEPING";
    }else if(state==64){
        *result = "DEAD";
    }else if(state==128){
        *result = "WAKEKILL";
    }else if(state==256){
        *result = "WAKING";
    }else if(state==512){
        *result = "PARKED";
    }else if(state==1024){
        *result = "NOLOAD";
    }else if(state==2048){
        *result = "NEW";
    }else if(state==4096){
        *result = "STATE_MAX";
    }else if(state==1026){
        *result = "ZOMBIE";
    }else{
        *result = "UNKNOWN";
    }

}
//print numbers of proccess in different states
void print_numbers_state(struct task_struct *t){
    struct list_head *pos;
    struct task_struct *p;
    int zombie = 0;
    int running = 0;
    int interruptible = 0;
    int uninterruptible = 0;
    int dead = 0;
    int wakekill = 0;
    int waking = 0;
    int new = 0;
    int parked = 0;
    int noload = 0;
    int unknown = 0;
    int state_max = 0;
    list_for_each(pos, &t->tasks){
        p = list_entry(pos,struct task_struct,tasks);
        long state = p->__state;
        if(state==0){
            running++;
        }else if(state==1){
            interruptible++;
        }else if(state==2){
            uninterruptible++;
        }else if(state==64){
            dead++;
        }else if(state==128){
            wakekill++;
        }else if(state==256){
            waking++;
        }else if(state==512){
            parked++;
        }else if(state==1024){
            noload++;
        }else if(state==2048){
            new++;
        }else if(state==4096){
            state_max++;
        }else if(state==1026){
            zombie++;
        }else{
            unknown++;
        }
    }
    length += sprintf(read_buf + length,"running:%d zombie:%d interruptible:%d uninterruptible:%d dead:%d new:%d parked:%d state_max:%d wakekill:%d noload:%d waking:%d unknown %d \n", running, zombie, interruptible, uninterruptible, dead, new, parked, state_max, wakekill, noload, waking, unknown);
}

ssize_t write_proc(struct file* filp, const char __user * buffer, size_t len, loff_t* offset){
printk("Call write\n");
int pid_read;
int numbers;
if(copy_from_user(write_buf,buffer,len)){
return -ENOMEM;
}
numbers = sscanf(write_buf,"%d",&pid_read);
if(numbers!=1){
return -EFAULT;
}
pid = pid_read;
int new_len = strlen(write_buf);
*offset = new_len;
printk("Chosen pid is %d\n", pid);
return new_len;
}

ssize_t read_proc( struct file *file, char __user *buff, size_t count, loff_t *ppos){
    static int read_status = 0;
    struct task_struct *task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    if(!task){
        copy_to_user(buff,"Nothing Found\n",15 );
        read_status = 15;
        return 0;
    } else {
    printk("Now read begins\n");

    int number = count_proccess(task);
    length += sprintf(buff + length,"Numbers of proccesses:%d\n",number);
    print_numbers_state(task);

    struct cpu_itimer* it_ITIMER_PROF = &task->signal->it[0];
    struct cpu_itimer* it_ITIMER_VIRTUAL = &task->signal->it[1];
    const char fields_values_str[WRITEBUF];
    const char format_answer[] = "For ITIMER_PROF Expires: [%d]\nIncr: [%d]\nFor ITIMER_VIRTUAL Expires: [%d], Incr: [%d]\\n";
    size_t string_size = snprintf(NULL, 0, format_answer, it_ITIMER_PROF->expires, it_ITIMER_PROF->incr, it_ITIMER_VIRTUAL->expires, it_ITIMER_VIRTUAL->incr);
    snprintf(fields_values_str, string_size, format_answer, it_ITIMER_PROF->expires, it_ITIMER_PROF->incr, it_ITIMER_VIRTUAL->expires, it_ITIMER_VIRTUAL->incr);
    copy_to_user(buff, fields_values_str, string_size);
    read_status = string_size;

//struct timerqueue_node tqn = task->signal->real_timer.node;
//struct timerqueue_head tqh = task->posix_cputimers.bases[0].tqhead;
//const char fields_values_str1[WRITEBUF];
//const char format_answer1[] = "Timerqueue node expires: %u\nTimer PID: %u\nTimerqueue head pointer: %x\n";
//size_t string_size_t = snprintf(NULL, 0, format_answer1, tqn.expires, pid, &tqh) + 1;
//snprintf(fields_values_str1, string_size_t, format_answer1, tqn.expires, pid, &tqh);
//    copy_to_user(buff, fields_values_str1, string_size_t);
//    read_status = string_size;

}
    if (*ppos >0 || count < READBUF)
        return 0;
    printk("Read finished\n");
    return read_status;
}

struct proc_ops operations = {
        .proc_read = read_proc,
        .proc_write = write_proc,
};

int __init task_init (void){
    proc_dir = proc_mkdir("lab2", NULL);
    proc_file = proc_create("lab_info",0666,proc_dir,&operations);
    return 0;
}

void __exit task_exit(void){
    if(proc_file){
        proc_remove(proc_file);
    }
    if(proc_dir){
        proc_remove(proc_dir);
    }
    printk("Finished\n");
}


module_init(task_init);
module_exit(task_exit);