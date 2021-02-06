#pragma once
#include <vector>
#include "contact.h"

namespace morph{ namespace softmats{
/**
 * Manages the list of collisions
 * 
 * @author Alejandro Jimenez Rodriguez
 */
class ContactList{
private:
    vector<Contact*> contacts;
public:
    
    ContactList();

    Contact* findContact( Body* A, Body* B );
    void push( Body* A, Body* B, Collision* c );
    int count();
    bool isEmpty();
    void clear();
    vector<Contact*>& getContacts();
    void prune();
    double getContactArea( bool includefloor );
    void updateReceptors();
    // Removes and returns the first collision in the queue
    // Collision* pop();
};

}}