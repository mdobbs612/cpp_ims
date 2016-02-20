#pragma once


#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h> /*sleep*/
#include <pthread.h> 
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>




#define BUFFER_LEN 250
#define DEFAULT_PORT "27012"
#define MAX_USERS 20
#define MAX_CLIENTS 20

using namespace std;


enum ACTIVE_STATUS { INACTIVE, ACTIVE };

// NOT = user2 is not user1's friend
// REQUESTED = user1 has already requested user2
// PENDING = user1 must answer user2's reqest
// YES = user1 and user2 are friends 
enum FRIEND_STATUS { NOT, REQUESTED, PENDING, YES };

class User {
	const char *username;
	int index;
	ACTIVE_STATUS active_status;

public:
	User(const char *name, int ind) : username(name), index(ind) { active_status = INACTIVE; }
	int get_index() { return index; }
	const char *get_name() { return username; }
};


class Database {
	int num_users;
	User *users[MAX_USERS];
	FRIEND_STATUS friend_matrix[MAX_USERS][MAX_USERS];

public:
	Database() { num_users = 0; }

	void insert_user(const char *name, int ind);
	int get_num_users() { return num_users; }

	const char *name_by_index(int ind);
	int index_by_name(const char *name);

	void request_friend(int user_ind, const char *friend_name);
	void accept_friend(int user_ind, const char *friend_name);
	

	FRIEND_STATUS friend_status(int user_ind, const char *friend_name);

	User *get_user_by_name(const char *name);
	User *get_user_by_index(int ind);

};


class Client {
	int logged_in; //Boolean for whether the client has a user logged in
	User *user_bound; // The user that is connected to server through this client (only after login success)
	pthread_t id; // thread id
	int client_socket;

public:
	
	int is_logged_in() { return logged_in; }
	pthread_t get_id() { return id; }
	int get_client_socket() { return client_socket; }
	const char *get_username() { return user_bound->get_name(); }
	void connect_user(User *usr) { user_bound = usr; logged_in = 1; }
	void logout_user() { user_bound = NULL; logged_in = 0; }


	Client() {
		id = -1;
		client_socket = -1;
	}

	Client(pthread_t thread_id, int s) { id = thread_id; client_socket = s; logged_in = 0; }

};

class Clients {
	int num_clients;
	Client **clients;

public:
	Clients() { num_clients = 0; clients = new Client*[MAX_CLIENTS]; };
	int new_client(Client *client) { clients[num_clients] = client; num_clients++; return 0; }
};

const char *ParseClientString(const char *buffer, Client *client);

#define MAX_IM_LEN 200
#define MAX_NAME_LEN 20

// ACT: An action is taken
// CONFIRM: Confirming that an act has been successful
// STATUS: Telling the user the status of another user.
// ERROR: Something went wrong
enum MSG_TYPE { ACTION = '0', CONFIRM = '1', STATUS = '2', ERR_MSG = '3' };

// REGISTER: Register user with the server --- does not list them as online yet
// LOGIN/LOGOUT: Changes user's online status
// REQ/REM/ACPT/DENY_FRIEND: Alter's user's relationship to other player
// LIST_FRIENDS: Prints lit of user's friends for the user, including statuses and pendings
// MSG_FRIEND: Sends an IM to a specified friend of the user
enum ACTION_TYPE : char {
	REGISTER = '0',
	LOGIN = '1',
	LOGOUT = '2',
	ADD_FRIEND = '3',
	REM_FRIEND = '4',
	STATUS_ALL = '5',
	STATUS_ONE = '6',
	MSG_FRIEND = '7', 
  ACPT_FRIEND = '8'
};

enum ERROR_TYPE {
	USER_ALREADY_EXISTS = '0',
	USER_DNE = '1',
	INVALID_NAME = '2',
	NOT_LOGGED_IN = '3',
	ALREADY_LOGGED_IN = '4',
  ALREADY_REQUESTED = '5',
  ALREADY_FRIENDS = '6'
};

// This is what will be sent to and from the clients.
class Msg {
	MSG_TYPE msg_type;
	const char *username; //required for some 9

public:
	//char *msg_to_str();
	//Msg (char *str);
	Msg(MSG_TYPE t, const char *name);
	MSG_TYPE get_msg_type() { return msg_type; }

	const char *get_username() { return username; }
};

class Action_Msg : public Msg {
	ACTION_TYPE action_type;
	const char *target_name;
	const char *message;				// for if the action is a message to send, empty otherwise

public:
	Action_Msg(ACTION_TYPE typ, const char *name, const char *target_name, const char *msg);
	ACTION_TYPE get_type() { return action_type; }
	const char *get_target() { return target_name; }
};

class Confirm_Msg : public Msg {
	ACTION_TYPE action_type;	// Tells the message which action it is confirming has happened
	const char *target_name; // Does this need username????

public:
	Confirm_Msg(ACTION_TYPE typ, const char *name, const char *target);
	ACTION_TYPE get_type() { return action_type; }
	const char *get_target() { return target_name; }

	const char *serialize();
};

class Status_Msg : public Msg {
	const char *target_name;						// In this case, this is the username of the person whose status is being reported
	ACTIVE_STATUS active_status;
	FRIEND_STATUS friend_status;

public:
	Status_Msg(const char *target, const char *name, ACTIVE_STATUS act, FRIEND_STATUS frnd);
	const char *get_target() { return target_name; }
	ACTIVE_STATUS get_active_status() { return active_status; }
	FRIEND_STATUS get_friend_status() { return friend_status; }

	const char *serialize();
};

class Err_Msg : public Msg {
	ERROR_TYPE error_type;

public:
	Err_Msg(ERROR_TYPE err, const char *name);

	const char *serialize();
};

void ParseMsg(Msg msg);
void ParseAction_Msg(Action_Msg msg);
void ParseConfirm_Msg(Confirm_Msg msg);
void ParseErr_Msg(Err_Msg msg);

const char *SerializeMsg(Msg msg, const char *buffer);

extern Database server_database;
