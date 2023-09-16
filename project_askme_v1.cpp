//#include <bits/stdc++.h>	// includes ALL
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>
using namespace std;

/////////////////////////////// Helper Methods ///////////////////////////////
vector<string> ReadFileLines(string path) {
	vector<string> lines;

	fstream file_handler(path.c_str());

	if (file_handler.fail()) {
		cout << "\n\nERROR: Can't open the file\n\n";
		return lines;
	}
	string line;

	while (getline(file_handler, line)) //when the getline function reaches the end of the file it returns false.
	{
		if (line.size() == 0)
			continue;
		lines.push_back(line);
	}

	file_handler.close();
	return lines;
}

void WriteFileLines(string path, vector<string> lines, bool append = true) {
	auto status = ios::in | ios::out | ios::app;

	if (!append)
		status = ios::in | ios::out | ios::trunc;	// overwrite

	fstream file_handler(path.c_str(), status);

	if (file_handler.fail()) {
		cout << "\n\nERROR: Can't open the file\n\n";
		return;
	}
	for (auto line : lines)
		file_handler << line << "\n";

	file_handler.close();
}

vector<string> SplitString(string line, string delimiter = ",") {
	vector<string> strs;

	int pos = 0;
	string substr;
	while ( (int)line.find(delimiter) != -1)  
	{
		pos=(int) line.find(delimiter); //find() returns an unsigned value so always use the conversion (int)
		substr = line.substr(0, pos);
		strs.push_back(substr);
		line.erase(0, pos + delimiter.length());
	}
	strs.push_back(line); //push back the remainder of the line
	return strs;
}

int ToInt(string str) {
	istringstream iss(str);
	int num;
	iss >> num;

	return num;
}

int ReadInt(int low, int high) {
	cout << "\nEnter number in range " << low << " - " << high << ": ";
	int value;

	cin >> value;

	if (low <= value && value <= high)
		return value;

	cout << "ERROR: invalid number...Try again\n";
	return ReadInt(low, high);
}

int ShowReadMenu(vector<string> choices) {
	cout << "\nMenu:\n";
	for (int ch = 0; ch < (int) choices.size(); ++ch) {
		cout << "\t" << ch + 1 << ": " << choices[ch] << "\n";
	}
	return ReadInt(1, choices.size()); // Enter number in range low - high :
}
//////////////////////////////////////////////////////////////

struct Question {
	int question_id;
	// To support thread. Each question look to a parent question
	// -1 No parent (first question in the thread)
	int parent_question_id;
	int from_user_id;
	int to_user_id;
	int is_anonymous_questions;	// 0 or 1
	string question_text;
	string answer_text;			// empty = not answered

	Question() {
		question_id = parent_question_id = from_user_id = to_user_id = -1;
		is_anonymous_questions = 1;
	}

	Question(string line) {
		vector<string> substrs = SplitString(line);
		assert(substrs.size() == 7);

		question_id = ToInt(substrs[0]);
		parent_question_id = ToInt(substrs[1]);
		from_user_id = ToInt(substrs[2]);
		to_user_id = ToInt(substrs[3]);
		is_anonymous_questions = ToInt(substrs[4]);
		question_text = substrs[5];
		answer_text = substrs[6];
	}

	string ToString() {
		ostringstream oss;
		oss << question_id << "," << parent_question_id << "," << from_user_id << "," << to_user_id << "," << is_anonymous_questions << "," << question_text << "," << answer_text;

		return oss.str();
	}

	void PrintToQuestion() //for only one question
	{
		string prefix = "";

		if (parent_question_id != -1) //as long as it is not a parent question
			prefix = "\tThread: ";

		cout << prefix << "Question Id (" << question_id << ")";
		if (!is_anonymous_questions)
			cout << " from user id(" << from_user_id << ")";
		cout << "\t Question: " << question_text << "\n";

		if (answer_text != "")
			cout << prefix << "\tAnswer: " << answer_text << "\n";
		cout << "\n";
	}

	void PrintFromQuestion() { //for only one question
		cout << "Question Id (" << question_id << ")";
		if (!is_anonymous_questions) //if you don't allow others to see your question_id, then you will not be able to see theirs.
			cout << " !AQ";

		cout << " to user id(" << to_user_id << ")";
		cout << "\t Question: " << question_text;

		if (answer_text != "")
			cout << "\tAnswer: " << answer_text << "\n";
		else
			cout << "\tNOT Answered YET\n";
	}

	void PrintFeedQuestion() {
		if (parent_question_id != -1)
			cout << "Thread Parent Question ID (" << parent_question_id << ") ";

		cout << "Question Id (" << question_id << ")";
		if (!is_anonymous_questions)
			cout << " from user id(" << from_user_id << ")";

		cout << " To user id(" << to_user_id << ")";

		cout << "\t Question: " << question_text << "\n";
		if (answer_text != "") 
			cout << "\tAnswer: " << answer_text <<"\n";
	}

};

struct User {
	int user_id;		// internal system ID
	string user_name;
	string password;
	string name;
	string email;
	int allow_anonymous_questions;	// 0 or 1

	vector<int> questions_id_from_me;
	// From question id to list of questions IDS on this question (thread questions) - For this user
	map<int, vector<int>> questionid_questionidsThead_to_map;

	User() {
		user_id = allow_anonymous_questions = -1;
	}

	User(string line) {
		vector<string> substrs = SplitString(line);
		assert(substrs.size() == 6);

		user_id = ToInt(substrs[0]);
		user_name = substrs[1];
		password = substrs[2];
		name = substrs[3];
		email = substrs[4];
		allow_anonymous_questions = ToInt(substrs[5]);
	}

	string ToString() {
		ostringstream oss;
		oss << user_id << "," << user_name << "," << password << "," << name << "," << email << "," << allow_anonymous_questions;

		return oss.str();
	}

	void Print() {
		cout << "User " << user_id << ", " << user_name << " " << password << ", " << name << ", " << email << "\n";
	}
};

struct QuestionsManager {
	// From question id to list of questions IDS on this question (thread questions) - All users
	map<int, vector<int>> questionid_questionidsThead_to_map;

	// Map the question id to question object. Let's keep one place ONLY with the object
	// When you study pointers, easier handling
	map<int, Question> questionid_questionobject_map;

	int last_id;

	QuestionsManager() {
		last_id = 0;
	}

	void LoadDatabase() {
		last_id = 0;
		questionid_questionidsThead_to_map.clear();
		questionid_questionobject_map.clear();

		vector<string> lines = ReadFileLines("C:/Users/eldoksh/Desktop/19 Project #3/questions.txt");
		for (auto &line : lines) 
		{
			Question question(line);
			last_id = max(last_id, question.question_id);  // last_id will be used in the AskQuestion function(will be assigned to new question_id )
			questionid_questionobject_map[question.question_id] = question;

			if (question.parent_question_id == -1)
				questionid_questionidsThead_to_map[question.question_id].push_back(question.question_id);
			else
				questionid_questionidsThead_to_map[question.parent_question_id].push_back(question.question_id);
		}
	}

	void FillUserQuestions(User &user) //will iterate over all the questions and if the question is from/to the current user then data will be uploaded into its data structures							  
	{
		user.questions_id_from_me.clear(); 
		user.questionid_questionidsThead_to_map.clear(); //this is a map of to_questions of the current_user

		for (auto &pair : questionid_questionidsThead_to_map) //this map for all users
		{	
			for (auto &question_id : pair.second) //question_id for each question that have been asked ever
			{		//  vector<Question

				// Get the question from the map. & means same in memory, DON'T COPY
				Question &question = questionid_questionobject_map[question_id];

				if (question.from_user_id == user.user_id)
					user.questions_id_from_me.push_back(question.question_id);

				if (question.to_user_id == user.user_id) 
				{
					if (question.parent_question_id == -1)
						user.questionid_questionidsThead_to_map[question.question_id].push_back(question.question_id);
					else
						user.questionid_questionidsThead_to_map[question.parent_question_id].push_back(question.question_id);
				}
			}
		}
	}

	void PrintUserToQuestions(User &user) {
		cout << "\n";

		if (user.questionid_questionidsThead_to_map.size() == 0)
			cout << "No Questions";

		for (auto &pair : user.questionid_questionidsThead_to_map) {		// pair<int, vector<Question>>
			for (auto &question_id : pair.second) {		//  vector<Question>

				// Get the question from the map. & means same in memory, DON'T COPY
				Question &question = questionid_questionobject_map[question_id];
				question.PrintToQuestion();
			}
		}
		cout << "\n";
	}

	void PrintUserFromQuestions(User &user) {
		cout << "\n";
		if (user.questions_id_from_me.size() == 0)
			cout << "No Questions";

		for (auto &question_id : user.questions_id_from_me) //  vector<Question_ids>
		{		

			// Get the question from the map. & means same in memory, DON'T COPY
			Question &question = questionid_questionobject_map[question_id];
			question.PrintFromQuestion();
		}
		cout << "\n";
	}

	// Used in Answering a question for YOU.
	// It can be any of your questions (thread or not)
	int ReadQuestionIdAny(User &user) {  //only questions sent to you
		int question_id;
		cout << "Enter Question id or -1 to cancel: ";
		cin >> question_id;

		if (question_id == -1)
			return -1;

		if (!questionid_questionobject_map.count(question_id)) {
			cout << "\nERROR: No question with such ID. Try again\n\n";
			return ReadQuestionIdAny(user);
		}
		Question &question = questionid_questionobject_map[question_id];

		if (question.to_user_id != user.user_id) { //you can only answer the questions sent to you.
			cout << "\nERROR: Invalid question ID. Try again\n\n";
			return ReadQuestionIdAny(user);
		}
		return question_id;
	}

	// Used to ask a question on a specific thread for whatever user(retrun the parent_question_id on which you will ask the question)
	int ReadQuestionIdThread(User &user) { 
		int question_id;
		cout << "For thread question: Enter Question id or -1 for new question: ";
		cin >> question_id;

		if (question_id == -1)
			return -1;

		if (!questionid_questionidsThead_to_map.count(question_id)) { //make sure that this id exist in questionid_questionidsThead_to_map
			cout << "No thread question with such ID. Try again\n";
			return ReadQuestionIdThread(user);
		}
		return question_id;
	}

	void AnswerQuestion(User &user) {
		int question_id = ReadQuestionIdAny(user);

		if (question_id == -1)
			return;

		Question &question = questionid_questionobject_map[question_id];

		question.PrintToQuestion();

		if (question.answer_text != "")
			cout << "\nWarning: Already answered. Answer will be updated\n";

		cout << "Enter answer: ";	// if user entered comma, system fails :)
		getline(cin, question.answer_text);	// read last enter
		getline(cin, question.answer_text);
	}
//انا شرحت الفنكشن دى فى فيديو
	void DeleteQuestion(User &user) {//only deletes questions to me
		int question_id = ReadQuestionIdAny(user);

		if (question_id == -1)
			return;

		vector<int> ids_to_remove;	// to remove from questionid_questionobject_map

		// Let's see if thread or not. If thread, remove all of it
		if (questionid_questionidsThead_to_map.count(question_id))  //thread(parent question)
		{
			ids_to_remove = questionid_questionidsThead_to_map[question_id];
			questionid_questionidsThead_to_map.erase(question_id); //remove the parent id it self
		} 
		else //we will remove a question_id from one of the vectores of questionid_questionidsThead_to_map
		{
			ids_to_remove.push_back(question_id);

			// let's find in which thread to remove. Consistency is important when have multi-view
			for (auto &pair : questionid_questionidsThead_to_map) //remove the child thread from questionid_questionidsThead_to_map
			{
				vector<int> &vec = pair.second;
				for (int pos = 0; pos < (int) vec.size(); ++pos) 
				{
					if (question_id == vec[pos]) 
					{
						vec.erase(vec.begin() + pos);
						break;
					}
				}
			}

		}

		for (auto id : ids_to_remove) { //remove the ids from questionid_questionobject_map 
			questionid_questionobject_map.erase(id);
		}
	}

	void AskQuestion(User &user, pair<int, int> to_user_pair) //pair of the int (to_user_id) and int (anonymous/nonanonymous)
	{
		Question question;

		if (!to_user_pair.second) {
			cout << "Note: Anonymous questions are not allowed for this user\n";
			question.is_anonymous_questions = 0;
		} else {
			cout << "Is anonymous questions?: (0 or 1): ";
			cin >> question.is_anonymous_questions;
		}

		question.parent_question_id = ReadQuestionIdThread(user);

		cout << "Enter question text: ";	// if user entered comma, system fails :)
		getline(cin, question.question_text);
		getline(cin, question.question_text);

		question.from_user_id = user.user_id;   
		question.to_user_id = to_user_pair.first; //to_user_pair.first is the to_user_id

		// What happens in 2 parallel sessions who asked question?
		// They are given same id. This is wrong handling :)
		question.question_id = ++last_id;

		questionid_questionobject_map[question.question_id] = question;

		if (question.parent_question_id == -1)
			questionid_questionidsThead_to_map[question.question_id].push_back(question.question_id);
		else
			questionid_questionidsThead_to_map[question.parent_question_id].push_back(question.question_id);
	}

	void ListFeed() {
		for (auto &pair : questionid_questionobject_map) 
		{
			Question &question = pair.second; //pair.second is an object of class question

			if (question.answer_text == "")
				continue;

			question.PrintFeedQuestion(); //print only question that hase an answer.
		}
	}

	void UpdateDatabase() {
		vector<string> lines;

		for (auto &pair : questionid_questionobject_map)
			lines.push_back(pair.second.ToString()); //pair.second is an object of class question

		WriteFileLines("C:/Users/eldoksh/Desktop/19 Project #3/questions.txt", lines, false);
	}
};

struct UsersManager {
	map<string, User> userame_userobject_map;
	User current_user;
	int last_id;

	UsersManager() {
		last_id = 0;
	}

	void LoadDatabase() {
		last_id = 0;
		userame_userobject_map.clear();

		vector<string> lines = ReadFileLines("C:/Users/eldoksh/Desktop/19 Project #3/users.txt");
		for (auto &line : lines) { // for each line a User object will be created
			User user(line);
			userame_userobject_map[user.user_name] = user; //map from user_name to the object user.
			last_id = max(last_id, user.user_id); //last_id will always hold the value of biggest user_id
												 // when a new user sign up we will give him last_id++ as a user_id
		}
	}

	void AccessSystem() {
		int choice = ShowReadMenu( { "Login", "Sign Up" });
		if (choice == 1)
			DoLogin();
		else
			DoSignUp();
	}

	void DoLogin() {
		LoadDatabase();	// in case user added from other parallel run

		while (true) {
			cout << "Enter user name & password: ";
			cin >> current_user.user_name >> current_user.password;

			if (!userame_userobject_map.count(current_user.user_name)) { //check wheather user_name exist in the map
				cout << "\nInvalid user name or password. Try again\n\n";
				continue;
			}
		User user_exist = userame_userobject_map[current_user.user_name]; //user_exist is the user that is already in the map

			if (current_user.password != user_exist.password) { //check wheather the password of current_user is the same as user_exist
				cout << "\nInvalid user name or password. Try again\n\n";
				continue;
			}
			current_user = user_exist; 	
			break;
		}
	}

	void DoSignUp() {
		while (true) {
			cout << "Enter user name. (No spaces): ";
			cin >> current_user.user_name;

			if (userame_userobject_map.count(current_user.user_name))
				cout << "Already used. Try again\n";
			else
				break;
		}
		cout << "Enter password: ";
		cin >> current_user.password;

		cout << "Enter name: ";
		cin >> current_user.name;

		cout << "Enter email: ";
		cin >> current_user.email;

		cout << "Allow anonymous questions? (0 or 1): ";
		cin >> current_user.allow_anonymous_questions;

		// What happens in 2 parallel sessions if they signed up?
		// They are given same id. This is wrong handling :)
		current_user.user_id = ++last_id;
		userame_userobject_map[current_user.user_name] = current_user;

		UpdateDatabase(current_user);
	}
	void UpdateDatabase(User &user) 
	{
		string line = user.ToString();
		vector<string> lines(1, line);
		WriteFileLines("C:/Users/eldoksh/Desktop/19 Project #3/users.txt", lines);
	}
	void ListUsersNamesIds() {
		for (auto &pair : userame_userobject_map)
			cout << "ID: " << pair.second.user_id << "\t\tName: " << pair.second.name << "\n";
	}

	pair<int, int> ReadUserId()  //retruns a pair of the "user_id" and the "allow_anonymous_questions"
	{
		int user_id;
		cout << "Enter User id or -1 to cancel: ";
		cin >> user_id;

		if (user_id == -1)
			return make_pair(-1, -1);

		for (auto &pair : userame_userobject_map) 
		{
			if (pair.second.user_id == user_id) //pair.second is an object of class User
				return make_pair(user_id, pair.second.allow_anonymous_questions);
		}

		cout << "Invalid User ID. Try again\n";
		return ReadUserId();
	}
};

struct AskMeSystem {
	UsersManager users_manager;
	QuestionsManager questions_manager;

	void LoadDatabase(bool fill_user_questions = false) 
	{
		users_manager.LoadDatabase();
		questions_manager.LoadDatabase();

		if (fill_user_questions)	// first time, waiting for login(there is no user loged in yet)
			questions_manager.FillUserQuestions(users_manager.current_user);//current_user is an object of class user
	}

	void run() {
		LoadDatabase();
		users_manager.AccessSystem(); //update the "users_manager.current_user" through login or signup.
		questions_manager.FillUserQuestions(users_manager.current_user);
	  /*current_user has a vector called questions_id_from_me
		and a map called questionid_questionidsThead_to_map 
		,FillUserQuestions() fills them with data */

		vector<string> menu;
		menu.push_back("Print Questions To Me");
		menu.push_back("Print Questions From Me");
		menu.push_back("Answer Question");
		menu.push_back("Delete Question");
		menu.push_back("Ask Question");
		menu.push_back("List System Users");
		menu.push_back("Feed");
		menu.push_back("Logout");

		while (true) {
			int choice = ShowReadMenu(menu);
			LoadDatabase(true);

			if (choice == 1)
				questions_manager.PrintUserToQuestions(users_manager.current_user);
			else if (choice == 2)
				questions_manager.PrintUserFromQuestions(users_manager.current_user);
			else if (choice == 3) {
				questions_manager.AnswerQuestion(users_manager.current_user);
				questions_manager.UpdateDatabase();
			} 
			else if (choice == 4) { //continue here
				questions_manager.DeleteQuestion(users_manager.current_user);
				// Let's build again (just easier, but slow)
				questions_manager.FillUserQuestions(users_manager.current_user);//update current_user questions
				questions_manager.UpdateDatabase();
			} 
			else if (choice == 5) {
				pair<int, int> to_user_pair = users_manager.ReadUserId();
				if (to_user_pair.first != -1) {
					questions_manager.AskQuestion(users_manager.current_user, to_user_pair);
					questions_manager.UpdateDatabase();
				}
			} 
			else if (choice == 6)
				users_manager.ListUsersNamesIds();
			else if (choice == 7)
				questions_manager.ListFeed();
			else
				break;
		}
		run();	// Restart again
	}
};

int main() {
	AskMeSystem service;
	service.run();

	return 0;
}


/*
101,-1,11,13,0,Should I learn C++ first or Java,I think C++ is a better Start
203,101,11,13,0,Why do you think so!,Just Google. There is an answer on Quora.
205,101,45,13,0,What about python?,
211,-1,13,11,1,It was nice to chat to you,For my pleasure Dr Mostafa
212,-1,13,45,0,Please search archive before asking,
300,101,11,13,1,Is it ok to learn Java for OOP?,Good choice
301,-1,11,13,0,Free to meet?,
302,101,11,13,1,Why so late in reply?,

13,mostafa,111,mostafa_saad_ibrahim,mostafa@gmail.com,1
11,noha,222,noha_salah,nono171@gmail.com,0
45,ali,333,ali_wael,wael@gmail.com,0

 */
