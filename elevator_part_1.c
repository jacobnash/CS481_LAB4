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
    pthread_cond_wait(p->cond, p->lock);
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
    Person *person_in_transit;
    for(;;)
    {

        pthread_mutex_lock(((Elevator*)arg)->es->lock);
        while (!((Queue*)((Elevator*)arg)->es->v)->count)
            pthread_cond_wait(((Queue*)((Elevator*)arg)->es->v)->cond, ((Elevator*)arg)->es->lock);
        // unlock the critial section.
        pthread_mutex_unlock(((Elevator*)arg)->es->lock);
        person_in_transit = (Person*)jval_v(dll_val(dll_first(((Queue*)((Elevator*)arg)->es->v)->passengers)));
        dll_delete_node(dll_first(((Queue*)((Elevator*)arg)->es->v)->passengers));
        //move the elevator.
        --((Queue*)((Elevator*)arg)->es->v)->count;
        person_in_transit->from == ((Elevator*)arg)->onfloor?:move_to_floor(((Elevator*)arg), person_in_transit->from);
        //open the door
        open_door(((Elevator*)arg));
        //add the people to the elevator
        person_in_transit->e = ((Elevator*)arg); 
        //signal the person
        pthread_mutex_lock(person_in_transit->lock);
        pthread_cond_signal(person_in_transit->cond);
        pthread_mutex_unlock(person_in_transit->lock);
        // have the elevator wait
        pthread_mutex_lock(((Elevator*)arg)->lock);
        pthread_cond_wait(((Elevator*)arg)->cond, ((Elevator*)arg)->lock);
        pthread_mutex_unlock(((Elevator*)arg)->lock);
        // close door 
        close_door(((Elevator*)arg));
        // elevator moves
        move_to_floor(person_in_transit->e,person_in_transit->to);
        // open the door once move has completed.
        open_door(((Elevator*)arg));
        // signal the person
        pthread_mutex_lock(person_in_transit->lock);
        pthread_cond_signal(person_in_transit->cond);
        pthread_mutex_unlock(person_in_transit->lock);
        // block the elevator
        pthread_mutex_lock(((Elevator*)arg)->lock);
        pthread_cond_wait(((Elevator*)arg)->cond, ((Elevator*)arg)->lock);
        pthread_mutex_unlock(((Elevator*)arg)->lock);
        // close the door
        close_door(((Elevator*)arg));
        // restart. 
    }
}
