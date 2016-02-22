#include "client.h"


Action_Msg::Action_Msg(ACTION_TYPE typ, const char * name, const char * target, const char * msg) : Msg(ACTION, name)
{
	action_type = typ;
	target_name = target;
	message = msg;
}

Confirm_Msg::Confirm_Msg(ACTION_TYPE typ, const char * name) : Msg(CONFIRM, name)
{
	action_type = typ;
	username = name;
}


Status_Msg::Status_Msg(const char * name, ACTIVE_STATUS act, FRIEND_STATUS frnd) : Msg(STATUS, name)
{
	username = name;
	active_status = act;
	friend_status = frnd;
}

Err_Msg::Err_Msg(ERROR_TYPE err, const char * name) : Msg(ERROR_MSG, name)
{
	error_type = err;
	username = name;
}

Msg::Msg(MSG_TYPE t, const char *name)
{
	msg_type = t;
	username = name;
}

const char * Action_Msg::serialize() {
  //cout << "SERIALIZING\n";
	char *buffer = new char[250];
  //cout << "buffa " << buffer << endl;
	buffer[0] = action_type;
  

	const char *usr = Msg::get_username();
  //cout << "USERNAME: " << usr << endl;
	int  len = strlen(usr);
	buffer[1] = (char)len + 1;
	for (int i = 0; i < 20; i++) {
    if (i < len) buffer[i + 2] = usr[i];
    else buffer [i + 2] = '#';
	}
	const char *target = get_target();
  //cout << "TARGET: " << target << endl;
  //cout << "buffa " << buffer << endl;
	len = strlen(target);
	buffer[22] = (char)len + 1;
	for (int i = 0; i < 20; i++) {
			if (i < len) buffer[23 + i] = target[i];
      else buffer[23 + i] = '#';
	}
	len = strlen(message);
	buffer[43] = (char)len + '0';
	for (int i = 0; i < 200; i++) {
			if (i < len) buffer[44 + i] = target[i];
      else buffer[44 + i] = '#';
	}
	buffer[244] = '\0';
  //cout << "buffa 22: " << buffer[22] << endl;
  //cout << "buffa " << buffer << endl;
	return buffer;
};



void ParseString(const char *buffer) {
	MSG_TYPE t = (MSG_TYPE)buffer[0];
  cout << "> Buffer is" << buffer << endl;
  cout << "> Type is " << t << endl;
	if (t == '1')  ParseConfirm(buffer);
	//else if (t == '2')  ParseStatus(buffer);
	else if (t == '3')  ParseError(buffer);
	else cout << "Weird String from Server\n";
}

void ParseConfirm(const char *buffer) {
	ACTION_TYPE t = (ACTION_TYPE)buffer[1];
	int len = (int)buffer[2] - 1;
	char *username = new char[len + 1];
	for (int i = 0; i < len; i++) {
		username[i] = buffer[3 + i];
	}
	username[len] = '\0';
	len = (int)buffer[23] - 1;
	char *target_name = new char[len + 1];
	for (int i = 0; i < len; i++) {
		target_name[i] = buffer[24 + i];
	}
	target_name[len] = '\0';

	switch (t) {
		case ('0'):
			cout << "SUCCESS: " << username << " registered\n";
			break;
		case ('1') :
			cout << "SUCCESS: " << username << " logged in\n";
			break;
    case ('2') : 
      cout << "SUCCESS: " << username << " logged out\n";
      break;
    case ('3') : 
      cout << "SUCCESS: " << target_name << " added\n";
      break;
    case ('8') : 
      cout << "SUCCESS: " << target_name << " accepted as friend\n";
      break;

	}
}

void ParseError(const char *buffer) {
	ERROR_TYPE t = (ERROR_TYPE)buffer[1];
	switch (t) {
		case ('0'):
			cout << "ERROR: User already exists\n";
			break;
		case ('1') :
			cout << "ERROR: User does not exist\n";
			break;
		case ('2') :
			cout << "ERROR: Invalid usename\n";
			break;
		case ('3') :
			cout << "ERROR: Client not logged in\n";
			break;
		case ('4') :
			cout << "ERROR: Client already logged in\n";
			break;
    case ('5') :
			cout << "ERROR: You already requested them\n";
			break;
    case ('6') :
			cout << "ERROR: You're already friends\n";
			break;
    case ('7') :
			cout << "ERROR: You can't be friends with yourself, sorry!\n";
			break;
    case ('8') :
			cout << "ERROR: You are not friends at the moment\n";
			break;
		/*case ('2') :
			cout << "ERROR: \n";
			break;
		case ('2') :
			cout << "ERROR: \n";
			break;
		case ('2') :
			cout << "ERROR: \n";
			break;*/

	}
}


/*
const char * SerializeMsg(Msg *msg, char * buffer)
{
	MSG_TYPE t;
	t = msg->get_msg_type();
	buffer[0] = t;

	if (t == 0) {
		//buffer = SerializeAction_Msg((Action_Msg *)msg, buffer + 1);
		Action_Msg *a_msg = (Action_Msg *)msg;
		buffer = a_msg->serialize();

	}
	else if (t == 1) {
		//buffer = SerializeConfirm_Msg((Confirm_Msg *)msg, buffer + 1);

		Confirm_Msg *c_msg = (Confirm_Msg *)msg;
		buffer = c_msg->serialize(buffer);
	}
	else if (t == 2) {
		//buffer = SerializeStatus_Msg((Status_Msg *)msg, buffer + 1);
		Status_Msg *s_msg = (Status_Msg *)msg;
		buffer = s_msg->serialize(buffer);

	}
	else if (t == 3) {
		//buffer = SerializeErr_Msg((Err_Msg *)msg, buffer + 1);

		Err_Msg *e_msg = (Err_Msg *)msg;
		buffer = e_msg->serialize(buffer);
	}

	else { return "error"; } //raise error somehow later

	return buffer;

}
*/
