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

typedef struct {
    pthread_cond_t *cond;    
    Dllist passengers;
    int count;
} Queue;

void initialize_simulation(Elevator_Simulation *es)
{
    Queue *list = mmalloc(Queue, 1);
    list->passengers = new_dllist();
    dll_empty(list->passengers);
    list->cond = mmalloc(pthread_cond_t, 1);
    pthread_cond_init(list->cond, NULL);
    list->count = 0;
    (*es).v = list; 
}

void initialize_elevator(Elevator *e)
{

}

void initialize_person(Person *e)
{

}

void wait_for_elevator(Person *p)
{
    // this is a critical section
    pthread_mutex_lock(p->es->lock);
    //append the list
    ((Queue*)p->es->v)->count++;
    dll_append(((Queue*)p->es->v)->passengers, new_jval_v(p));
    //signal the elevators that some one is infact there;
    pthread_cond_signal(((Queue*)p->es->v)->cond);
    // remove the critical section
    pthread_mutex_unlock(p->es->lock);
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
    int tempe;
    Person *person_in_transit;
    Dllist  temp, loading;
    loading = new_dllist();
    Elevator *this_elevator = (Elevator*)arg;//tired of typeing that 
    Queue *queue = (Queue*)this_elevator->es->v;
    for(;;)
    {
        //lock and wait for a signal. 
        pthread_mutex_lock(this_elevator->es->lock);
        while (!queue->count)
            pthread_cond_wait(queue->cond, this_elevator->es->lock);
        pthread_mutex_unlock(this_elevator->es->lock);
        //elevator is at the bottom of the building. 
        if(this_elevator->onfloor == 1)
        {
            //while the elevator is not on the top floor scroll up.
            while( this_elevator->onfloor != this_elevator->es->nfloors)
            {
                // if the queue of passangers are not empty
                if(!dll_empty(queue->passengers))
                {   // see of there are any passagers on the floor that the elevator is on. 
                    pthread_mutex_lock(this_elevator->es->lock);
                    dll_traverse(temp, queue->passengers)
                    {
                        //lock it out and add them to the loading queue
                        if (((Person*)jval_v(dll_val(temp)))->from == this_elevator->onfloor && ((((Person*)jval_v(dll_val(temp)))->to - ((Person*)jval_v(dll_val(temp)))->from) > 0)){
                            dll_append(loading, dll_val(temp));
                            modify_dll_delete_node(temp);
                            --queue->count;
                        }
                    }
                    pthread_mutex_unlock(this_elevator->es->lock);
                }
                // check the elevator to see of it is empty or not. 
                if(!dll_empty(this_elevator->people))
                {
                    //check if they are on the floor that the should be.
                    dll_traverse(temp, this_elevator->people)
                    {
                        if(this_elevator->onfloor == ((Person*)jval_v(dll_val(temp)))->to)
                        {
                            //get them off the eleevator. 
                            if (!this_elevator->door_open)
                            {
                                open_door(this_elevator);
                            }
                            pthread_mutex_lock(((Person*)jval_v(dll_val(temp)))->lock);
                            pthread_cond_signal( ((Person*)jval_v(dll_val(temp)))->cond);
                            pthread_mutex_unlock(((Person*)jval_v(dll_val(temp)))->lock); 
                        }
                        // if the door is open on that floor you should probably close it. 
                        // My grandfather used to tall me, "jake, you don't live in a barn."
                    }
                }
                // if there was some one that needed to be loaded on follow
                if(!dll_empty(loading))
                {
                    // open the door.
                    if (!this_elevator->door_open)
                    {
                        open_door(this_elevator);
                    }
                    // there may be many people loading. 
                    dll_traverse(temp, loading)
                    {
                        //do what is needed to get them on the elevator 
                        ((Person*)jval_v(dll_val(temp)))->e = this_elevator;
                        pthread_mutex_lock(((Person*)jval_v(dll_val(temp)))->lock);
                        pthread_cond_signal( ((Person*)jval_v(dll_val(temp)))->cond);
                        pthread_mutex_unlock(((Person*)jval_v(dll_val(temp)))->lock);
                        modify_dll_delete_node(temp);
                    }
                }
                if(this_elevator->door_open)
                {
                    pthread_mutex_lock(this_elevator->lock);
                    pthread_cond_wait(this_elevator->cond, this_elevator->lock);
                    pthread_mutex_unlock(this_elevator->lock);
                    close_door(this_elevator);
                }
                // this is where we go to the next floor. 
                tempe = this_elevator->onfloor;
                if(tempe++ < this_elevator->es->nfloors){
                    move_to_floor(this_elevator, tempe); 
                }
            }
        }
        if(this_elevator->onfloor == this_elevator->es->nfloors)
        {
            //while the elevator is not on the top floor scroll up.
            while( this_elevator->onfloor != 1)
            {
                // if the queue of passangers are not empty
                if(!dll_empty(queue->passengers))
                {   // see of there are any passagers on the floor that the elevator is on. 
                    pthread_mutex_lock(this_elevator->es->lock);
                    dll_traverse(temp, queue->passengers)
                    {
                        //lock it out and add them to the loading queue
                        if (((Person*)jval_v(dll_val(temp)))->from == this_elevator->onfloor && ((((Person*)jval_v(dll_val(temp)))->to - ((Person*)jval_v(dll_val(temp)))->from) < 0)){
                            dll_append(loading, dll_val(temp));
                            modify_dll_delete_node(temp);
                            --queue->count;
                        }
                    }
                    pthread_mutex_unlock(this_elevator->es->lock);
                }
                // check the elevator to see of it is empty or not. 
                if(!dll_empty(this_elevator->people))
                {
                    //check if they are on the floor that the should be.
                    dll_traverse(temp, this_elevator->people)
                    {
                        if(this_elevator->onfloor == ((Person*)jval_v(dll_val(temp)))->to)
                        {
                            //get them off the eleevator. 
                            if (!this_elevator->door_open)
                            {
                                open_door(this_elevator);
                            }
                            pthread_mutex_lock(((Person*)jval_v(dll_val(temp)))->lock);
                            pthread_cond_signal( ((Person*)jval_v(dll_val(temp)))->cond);
                            pthread_mutex_unlock(((Person*)jval_v(dll_val(temp)))->lock);

                        }
                        // if the door is open on that floor you should probably close it. 
                        // My grandfather used to tall me, "jake, you don't live in a barn."
                    }
                }
                // if there was some one that needed to be loaded on follow
                if(!dll_empty(loading))
                {
                    // open the door.
                    if (!this_elevator->door_open)
                    {
                        open_door(this_elevator);
                    }
                    // there may be many people loading. 
                    dll_traverse(temp, loading)
                    {
                        //do what is needed to get them on the elevator 
                        ((Person*)jval_v(dll_val(temp)))->e = this_elevator;
                        pthread_mutex_lock(((Person*)jval_v(dll_val(temp)))->lock);
                        pthread_cond_signal( ((Person*)jval_v(dll_val(temp)))->cond);
                        pthread_mutex_unlock(((Person*)jval_v(dll_val(temp)))->lock);
                        modify_dll_delete_node(temp);
                    }
                }
                if(this_elevator->door_open)
                {
                    pthread_mutex_lock(this_elevator->lock);
                    pthread_cond_wait(this_elevator->cond, this_elevator->lock);
                    pthread_mutex_unlock(this_elevator->lock);
                    close_door(this_elevator);
                }
                // this is where we go to the next floor. 
                tempe = this_elevator->onfloor;
                if(tempe-- > 1){
                    move_to_floor(this_elevator, tempe); 
                }
            }
        }
    }
}
