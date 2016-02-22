#include "server.h"

void Database::insert_user(const char * name, int ind)
{
	User *user;
	user = new User(name, ind);

	users[ind] = user;

	int i;

	for (i = 0; i < ind; i++) {
		friend_matrix[ind][i] = NOT;
		friend_matrix[i][ind] = NOT;
	}

	num_users++;
}

const char * Database::name_by_index(int ind)
{
	return users[ind]->get_name();
}

int Database::index_by_name(const char * name)
{
	int i;

	for (i = 0; i < num_users; i++) {
		if (strcmp(users[i]->get_name(), name) == 0) {
			return users[i]->get_index();
		}
	}

	return -1;
}

void Database::request_friend(int user_ind, const char * friend_name)
{
	int friend_ind = index_by_name(friend_name);

	friend_matrix[user_ind][friend_ind] = REQUESTED;
	friend_matrix[friend_ind][user_ind] = PENDING;

	// ACK messages, Show the request up if they're online

}


void Database::remove_friend(int user_ind, const char * friend_name)
{
	int friend_ind = index_by_name(friend_name);

	friend_matrix[user_ind][friend_ind] = NOT;
	friend_matrix[friend_ind][user_ind] = NOT;

	// ACK messages, Show the request up if they're online

}


void Database::accept_friend(int user_ind, const char * friend_name)
{
	int friend_ind = index_by_name(friend_name);

	if (friend_matrix[user_ind][friend_ind] == PENDING) {
		friend_matrix[user_ind][friend_ind] = YES;
		friend_matrix[friend_ind][user_ind] = YES;
	}
	//ACk stuff, ya know.
	//send error if not requested or if already friend
}

FRIEND_STATUS Database::friend_status(int user_ind, const char *friend_name)
{
	// might want to do a check if the index is in the bounds I think. 
	int friend_ind = index_by_name(friend_name);

	return friend_matrix[user_ind][friend_ind];
}

User * Database::get_user_by_name(const char * name)
{
	for (int i = 0; i < num_users; i++) {
		if (strcmp(users[i]->get_name(), name) == 0) { return users[i]; }
	}
	cout << "ERROR: Couldn't find user " << name << endl;
	return new User();
}

User * Database::get_user_by_index(int ind)
{
	if (ind < num_users) { return users[ind]; }
	else {
		cout << "Error: Couldn't get user " << ind << endl;
		return nullptr;
	}
}
