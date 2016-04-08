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
    Person *person_in_transit;
    Dllist  temp, loading;
    loading = new_dllist();
    Elevator *this_elevator = (Elevator*)arg;//tired of typeing that 
    Queue *queue = (Queue*)this_elevator->es->v;
    for(;;)
    {
        pthread_mutex_lock(this_elevator->es->lock);
        while (!queue->count)
            pthread_cond_wait(queue->cond, this_elevator->es->lock);
        pthread_mutex_unlock(this_elevator->es->lock);

        if(this_elevator->onfloor == 1)
        {
            while( this_elevator->onfloor != this_elevator->es->nfloors)
            {
                if(!dll_empty(queue->passengers))
                {   
                    dll_traverse(temp, queue->passengers)
                    {
                        pthread_mutex_lock(this_elevator->es->lock);
                        if (((Person*)jval_v(dll_val(temp)))->from == this_elevator->onfloor){
                            dll_append(loading, dll_val(temp));
                            modify_dll_delete_node(temp);
                            --queue->count;
                        }
                        pthread_mutex_unlock(this_elevator->es->lock);
                    }
                }
                if(!dll_empty(loading))
                {
                    open_door(this_elevator);


                    dll_traverse(temp, loading)
                    {
                        ((Person*)jval_v(dll_val(temp)))->e = this_elevator;
                        pthread_mutex_lock(((Person*)jval_v(dll_val(temp)))->lock);
                        pthread_cond_signal( ((Person*)jval_v(dll_val(temp)))->cond);
                        pthread_mutex_unlock(((Person*)jval_v(dll_val(temp)))->lock); 


                    }
                    dll_traverse(temp, loading)modify_dll_delete_node(temp);

                    pthread_mutex_lock(this_elevator->lock);
                    pthread_cond_wait(this_elevator->cond, this_elevator->lock);
                    pthread_mutex_unlock(this_elevator->lock);

                    close_door(this_elevator);
                }
                if(!dll_empty(this_elevator->people))
                {
                    dll_traverse(temp, this_elevator->people){
                        if(this_elevator->onfloor == ((Person*)jval_v(dll_val(temp)))->to)
                        {
                            if (!this_elevator->door_open)
                            {
                                open_door(this_elevator);
                            }
                            pthread_mutex_lock(((Person*)jval_v(dll_val(temp)))->lock);
                            pthread_cond_signal( ((Person*)jval_v(dll_val(temp)))->cond);
                            pthread_mutex_unlock(((Person*)jval_v(dll_val(temp)))->lock); 
                        }
                        if(this_elevator->door_open)
                        {
                            pthread_mutex_lock(this_elevator->lock);
                            pthread_cond_wait(this_elevator->cond, this_elevator->lock);
                            pthread_mutex_unlock(this_elevator->lock);

                            close_door(this_elevator);
                        }
                    }

                }
                int tempe = this_elevator->onfloor;
                if(++tempe<= this_elevator->es->nfloors){
                    move_to_floor(this_elevator, tempe); 
                    fprintf(stderr, " %i %i", this_elevator->onfloor,tempe);
                }
            }

        }
        else if(this_elevator->onfloor == this_elevator->es->nfloors)
        {
            while( this_elevator->onfloor != 1)
            {
                if(!dll_empty(queue->passengers))
                {   
                    dll_traverse(temp, queue->passengers)
                    {
                        pthread_mutex_lock(this_elevator->es->lock);
                        if (((Person*)jval_v(dll_val(temp)))->from == this_elevator->onfloor){
                            dll_append(loading, dll_val(temp));
                            modify_dll_delete_node(temp);
                            --queue->count;
                        }
                        pthread_mutex_unlock(this_elevator->es->lock);
                    }
                }
                if(!dll_empty(loading))
                {
                    open_door(this_elevator);


                    dll_traverse(temp, loading)
                    {
                        ((Person*)jval_v(dll_val(temp)))->e = this_elevator;
                        pthread_mutex_lock(((Person*)jval_v(dll_val(temp)))->lock);
                        pthread_cond_signal( ((Person*)jval_v(dll_val(temp)))->cond);
                        pthread_mutex_unlock(((Person*)jval_v(dll_val(temp)))->lock); 


                    }
                    dll_traverse(temp, loading)modify_dll_delete_node(temp);

                    pthread_mutex_lock(this_elevator->lock);
                    pthread_cond_wait(this_elevator->cond, this_elevator->lock);
                    pthread_mutex_unlock(this_elevator->lock);

                    close_door(this_elevator);
                }
                if(!dll_empty(this_elevator->people))
                {
                    dll_traverse(temp, this_elevator->people){
                        if(this_elevator->onfloor == ((Person*)jval_v(dll_val(temp)))->to)
                        {
                            fprintf(stderr, " %i %i", this_elevator->onfloor,((Person*)jval_v(dll_val(temp)))->to);
                            if (!this_elevator->door_open)
                            {
                                open_door(this_elevator);
                            }
                            pthread_mutex_lock(((Person*)jval_v(dll_val(temp)))->lock);
                            pthread_cond_signal( ((Person*)jval_v(dll_val(temp)))->cond);
                            pthread_mutex_unlock(((Person*)jval_v(dll_val(temp)))->lock); 
                        }
                        if(this_elevator->door_open)
                        {
                            pthread_mutex_lock(this_elevator->lock);
                            pthread_cond_wait(this_elevator->cond, this_elevator->lock);
                            pthread_mutex_unlock(this_elevator->lock);

                            close_door(this_elevator);
                        }
                    }

                }
                move_to_floor(this_elevator, --this_elevator->onfloor);

            }

        }
    }
}
