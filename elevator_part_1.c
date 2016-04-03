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
} Queue;

void initialize_simulation(Elevator_Simulation *es)
{
    Queue *list = mmalloc(Queue, 1);
    list->passengers = new_dllist();
    dll_empty(list->passengers);
    list->cond = mmalloc(pthread_cond_t, 1);
    pthread_cond_init(list->cond, NULL);
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

    pthread_mutex_lock(p->lock);
    dll_append( ((Queue*)p->es->v)->passengers, new_jval_v(p));
    pthread_mutex_unlock(p->lock);
    pthread_cond_wait(p->cond, p->lock);
    // Signal the condition variable for blocking elevators. Block on the person’s condition variable.
    pthread_cond_signal(p->e->cond);
}

void wait_to_get_off_elevator(Person *p)
{
    //Unblock the elevator’s condition variable block on the person’s condition variable.
    // block on the persons condtional
    pthread_cond_wait(p->cond, p->lock);
    
}

void person_done(Person *p)
{
    //this is for line 49.
    pthread_cond_signal(p->e->cond);
}
//Each elevator is a while loop. 
//Check the global list and if it’s empty, block on the condition variable for blocking elevators. 
//When the elevator gets a person to service, it moves to the appropriate flo0or and opens its door. 
//It puts itself into the person’s e field, then signals the person and blocks until the person wakes it up. 
//When it wakes up, it goes to the person’s destination floor, opens its door, signals the person and blocks. 
//When the person wakes it up, it closes its door and re-executes its while loop.
//• Your elevator’s should call open door(), close door() from move to floor() appropriately. 
//All sleeping and printing should be done in elevator skeleton.c. 
//Thus, your final program that you hand in should not do any sleeping or any printing.
void *elevator(void *arg)
{
    for (;;)
    {
        Person *person_in_transit = (Person*)jval_v(dll_val(dll_first(((Queue*)((Elevator*)arg)->es->v)->passengers)));
        if (!person_in_transit){
            //pthread_mutex_lock(((Elevator*)arg)->lock);
        }
        else
        {
            person_in_transit = (Person*)jval_v(dll_val(dll_first(((Queue*)((Elevator*)arg)->es->v)->passengers)));
            dll_delete_node(dll_first(((Queue*)((Elevator*)arg)->es->v)->passengers));
            person_in_transit->e = ((Elevator*)arg); 
            person_in_transit->from == ((Elevator*)arg)->onfloor?:move_to_floor(((Elevator*)arg), person_in_transit->from);
            open_door(((Elevator*)arg));
            pthread_cond_signal(person_in_transit->cond);
            pthread_cond_wait(((Elevator*)arg)->cond, ((Elevator*)arg)->lock);
            pthread_mutex_unlock(((Elevator*)arg)->lock);
            close_door(((Elevator*)arg));
            move_to_floor(((Elevator*)arg),person_in_transit->to);
            open_door(((Elevator*)arg));
            pthread_cond_signal(person_in_transit->cond);
            pthread_cond_wait(((Elevator*)arg)->cond, ((Elevator*)arg)->lock);
            pthread_mutex_unlock(((Elevator*)arg)->lock);
            close_door(((Elevator*)arg));
        }
    }
    return NULL;
}
