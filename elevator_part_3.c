/* elevator_null.c
   Null solution for the elevator threads lab.
   Jim Plank
   CS560
   Lab 2
   January, 2009
   Jacob Nash 
   */

#include <stdio.h>
#include <pthread.h>
#include "elevator.h"
#include "stdlib.h"
// I stole this from you because its genius.
#define mmalloc(type, count) ( type * ) malloc ((count) * sizeof(type))
// this is needed to be globally but i can create a pointer to it.
int startfloor = 0;
typedef struct { 
    Dllist *on_elevator;
} Wait;
typedef struct {
    pthread_cond_t *cond;    
    Dllist *passengers;
    pthread_mutex_t *locks;
} Queue;
void initialize_simulation(Elevator_Simulation *es)
{

    Queue *list = mmalloc(Queue, 1);
    int i;
    // create a list for up and down. 
    list->passengers = mmalloc(Dllist, (es->nfloors*2));
    for(i =0; i < (es->nfloors *2); i++ )
        list->passengers[i] = new_dllist();
    list->cond = mmalloc(pthread_cond_t, 1);
    pthread_cond_init(list->cond, NULL);
    list->locks = mmalloc(pthread_mutex_t, es->nfloors);
    for(i = 0; i < es->nfloors; i++)
    {
        pthread_mutex_init(&list->locks[i], NULL);
    }

    (*es).v = list; 
}

void initialize_elevator(Elevator *e)
{
    int i;
    // create a get off queue. 
    Wait *on_elevator = mmalloc(Wait, 1);
    on_elevator->on_elevator = mmalloc(Dllist, e->es->nfloors);
    for ( i = 0; i < e->es->nfloors; i++)
        on_elevator->on_elevator[i] = new_dllist();
    e->v = on_elevator;
}

void initialize_person(Person *e)
{

}

void wait_for_elevator(Person *p)
{
    // this is a critical section
    Queue *queue = (Queue*)p->es->v;
    if((p->from < p->to))
    {//going up.
        pthread_mutex_lock(&((Queue*)p->es->v)->locks[p->from - 1]);
        //append the list
        dll_append(((Queue*)p->es->v)->passengers[(p->from - 1)], new_jval_v(p));
        // remove the critical section
        pthread_mutex_unlock(&((Queue*)p->es->v)->locks[p->from - 1]);
    } 
    else 
    {// going down
        pthread_mutex_lock(&((Queue*)p->es->v)->locks[p->from - 1]);
        //append the list
        dll_append(((Queue*)p->es->v)->passengers[(p->es->nfloors) + p->from - 1], new_jval_v(p));
        // remove the critical section
        pthread_mutex_unlock(&((Queue*)p->es->v)->locks[p->from - 1]);

    }
    // block on the persons varible
    pthread_mutex_lock(p->lock);
    // wait for signal.
    pthread_cond_wait(p->cond, p->lock);
    // unlock on signal.
    pthread_mutex_unlock(p->lock);
}

void wait_to_get_off_elevator(Person *p)
{
    //Unblock the elevator’s condition variable block on the person’s condition variable.
    pthread_mutex_lock(p->e->lock);
    pthread_cond_signal(p->e->cond);
    pthread_mutex_unlock(p->e->lock);
    // block on the persons condtional
    pthread_mutex_lock(p->lock);
    pthread_cond_wait(p->cond, p->lock);
    pthread_mutex_unlock(p->lock);

}

void person_done(Person *p)
{
    //signal to the elevator. 
    pthread_mutex_lock(p->e->lock);
    pthread_cond_signal(p->e->cond);
    pthread_mutex_unlock(p->e->lock);
}
void *elevator(void *arg)
{
    int d = 0; // 0 is up 1 is down. 
    int tempe;
    Elevator *this_elevator = (Elevator*)arg;//tired of typeing that 
    Dllist *this_elevator_queue = ((Wait*)this_elevator->v)->on_elevator;
    Queue *queue = (Queue*)this_elevator->es->v;
    Person *person_in_transit;
    for(;;)
    {
        //get off the elevator.
        while(this_elevator_queue[this_elevator->onfloor-1]->flink  != this_elevator_queue[this_elevator->onfloor -1])
        {
            if (!this_elevator->door_open)
            {
                open_door(this_elevator);
            }
            person_in_transit = (Person*)jval_v(this_elevator_queue[this_elevator->onfloor - 1]->flink->val);
            dll_delete_node(this_elevator_queue[this_elevator->onfloor - 1]->flink);

            pthread_mutex_lock(person_in_transit->lock);
            pthread_cond_signal(person_in_transit->cond);
            pthread_mutex_unlock(person_in_transit->lock);
            // have the elevator wait
            pthread_mutex_lock(this_elevator->lock);
            pthread_cond_wait(this_elevator->cond, this_elevator->lock);
            pthread_mutex_unlock(this_elevator->lock);

        }
        // get on the elevator.
        if (pthread_mutex_trylock(&queue->locks[this_elevator->onfloor - 1]) == 0)
        {
            // try to get the lock if you can't its probably because some one is already on the floor. 
            while(queue->passengers[d * this_elevator->es->nfloors + this_elevator->onfloor - 1]->flink != queue->passengers[d * this_elevator->es->nfloors + this_elevator->onfloor - 1])
            { //get on the elevator.
                if (!this_elevator->door_open)
                {
                    open_door(this_elevator);
                }
                person_in_transit = (Person*)jval_v(queue->passengers[d * this_elevator->es->nfloors +  this_elevator->onfloor - 1]->flink->val);
                person_in_transit->e = this_elevator;
                dll_append(this_elevator_queue[person_in_transit->to - 1], new_jval_v(person_in_transit));
                dll_delete_node(queue->passengers[d * this_elevator->es->nfloors + this_elevator->onfloor - 1]->flink);

                pthread_mutex_lock(person_in_transit->lock);
                pthread_cond_signal(person_in_transit->cond);
                pthread_mutex_unlock(person_in_transit->lock);
                // have the elevator wait
                pthread_mutex_lock(this_elevator->lock);
                pthread_cond_wait(this_elevator->cond, this_elevator->lock);
                pthread_mutex_unlock(this_elevator->lock);
            }
            pthread_mutex_unlock(&queue->locks[this_elevator->onfloor - 1]);
        }
        if (this_elevator->door_open)
        {
            close_door(this_elevator);
        }
        move_to_floor(this_elevator, (this_elevator->onfloor + ( 1 + (-2 * d)))); 
        if (this_elevator->onfloor == 1)
            d = 0;
        else if ( this_elevator->onfloor == this_elevator->es->nfloors)
            d = 1; 
        // check to see if there are peeple in any of the queues on the floor above.                
    }
}
