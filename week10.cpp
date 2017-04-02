/***********************************************************************
* Program:
*    Unit 3, ACLs
*    Brother Wilson, CS470
* Author:
*    Edgar Wright
* Summary: 
*    This program tracks a collection of student grades
*    Currently, it performs no authentication and furthermore
*    is so trivially hack-able that it is a joke.  Hahaha.  Why
*    is noone laughing?  OK, as you guess, your job in a collection
*    of assignments is to make this program secure.  Enjoy!
************************************************************************/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <cassert>
using namespace std;

// levels of secrecy
enum Control
{
      PUBLIC,
      STUDENT,
      GRADER,
      PROFESSOR
};

//Permissions definitions 
// 0 = no access
// 1 = read
// 2 = write
// 3 = both

struct Ace 
{
      const char *groupId;
      const char *fileId;
      const char *permissions[3];
};

struct User
{
      const char *name;
      const char *password;
      const char *userGroup;
      const char *fullName;
};
/******************************************
 * FILE
 * A single file resource 
 *****************************************/
struct File
{
      const char *filename; // where it is located
      const char *id; 
};

/****************************************
 * Files
 * All the files currently on the system
 ****************************************/
const File files[3] =
    {
        {"/home/cs470/week10/sam.txt", "1"},
        {"/home/cs470/week10/sue.txt", "2"},
        {"/home/cs470/week10/sly.txt", "3"}
    };

/*****************************************************************
 *****************************************************************
 *                       Student Grade class                     *
 *****************************************************************
 *****************************************************************/
struct Item
{
      float weight;
      float score;
};

/***************************************************
 * One instance of a student grade
 ***************************************************/
class StudentGrade
{
    public:
      StudentGrade(const File &files);
      ~StudentGrade();
      string getLetterGrade();               // output letter grade B+
      float getNumberGrade();                // integral number grade 88
      void displayScores(Ace* filePermissions, User userSignedIn); // display scores on screen
      void editScores(Ace* filePermissions, User userSignedIn);    // interactively edit score
      void setScore(int iScore, float score);
      float getScore(int iScore);
      void setWeight(int iScore, float weight);
      float getWeight(int iScore);
      string getName() { return name; };
      string getUserName() { return userName; };
      string getUserGroup() { return userGroup; };
    private:
      bool change;
      string name;         // student's name
      vector<Item> scores; // list of scores and weightings
      const char *filename;
      const char *currentFileId;
      string userName;
      string userGroup;
      void editScore(int); // edit one score
};

/**********************************************
 * SET SCORE
 **********************************************/
void StudentGrade::setScore(int iScore, float score)
{
      assert(iScore >= 0 && iScore < scores.size());
      scores[iScore].score = score;
      change = true;
}

/**********************************************
 * GET SCORE
 **********************************************/
float StudentGrade::getScore(int iScore)
{
      assert(iScore >= 0 && iScore < scores.size());
      return scores[iScore].score;
}

/****************************************
 * SET WEIGHT
 ****************************************/
void StudentGrade::setWeight(int iScore, float weight)
{
      assert(iScore >= 0 && iScore < scores.size());
      if (weight >= 0.0)
      {
            scores[iScore].weight = weight;
            change = true;
      }
}

/**********************************************
 * GET WEIGHT
 **********************************************/
float StudentGrade::getWeight(int iScore)
{
      assert(iScore >= 0 && iScore < scores.size());
      return scores[iScore].weight;
}

/***********************************************
 * STUDENT GRADE
 * constructor: read the grades from a file
 **********************************************/
StudentGrade::StudentGrade(const File &file) : change(false)
{
      filename = file.filename;
      currentFileId = file.id;
      assert(filename && *filename);

      // open file
      ifstream fin(filename);
      if (fin.fail())
            return;

      // read name
      getline(fin, name);

      // read scores
      Item item;
      while (fin >> item.score >> item.weight)
            scores.push_back(item);

      // close up shop
      fin.close();
}

/**************************************************
 * DESTRUCTOR
 * Write the changes to the file if anything changed
 *************************************************/
StudentGrade::~StudentGrade()
{
      assert(filename && *filename);

      if (!change)
            return;

      // open file
      ofstream fout(filename);
      if (fout.fail())
            return;

      // header is the students name
      fout << name << endl;

      // write the data
      for (int iScore = 0; iScore < scores.size(); iScore++)
            fout << scores[iScore].score
                 << "\t"
                 << scores[iScore].weight
                 << endl;

      // make like a tree
      fout.close();
}

/****************************************
 * Edit only one score.
 ***************************************/
void StudentGrade::editScore(int iScoreEdit)
{
      float userInput; // user inputed weight.

      assert(iScoreEdit >= 0 && iScoreEdit < scores.size());

      //
      // Score
      //

      // get new score
      cout << "Enter grade: ";
      cin >> userInput;

      // validate
      while (userInput > 100 || userInput < 0)
      {
            cout << "Invalid grade.  Select a number between 0 and 100: ";
            cin >> userInput;
      }
      setScore(iScoreEdit, userInput);

      //
      // Weight
      //

      // get the weight
      cout << "Enter the weight for the score or (0) for unchanged: ";
      cin >> userInput;

      // validate
      while (userInput > 1.0 || userInput < 0.0)
      {
            cout << "Invalid weight.  Select a number between 0 and 1: ";
            cin >> userInput;
      }
      if (userInput != 0.0)
            setWeight(iScoreEdit, userInput);

      return;
}

/*********************************************
 * Edit scores until user says he is done
 *******************************************/
void StudentGrade::editScores(Ace* filePermissions, User userSignedIn)
{
      // Give the user some feedback
      cout << "Editing the scores of "
           << name
           << endl;

      // display score list
      cout << "Score list\n";

          bool canWrite;

      for (int i = 0; i < sizeof(filePermissions) / sizeof(Ace); i++)
      {
            if (userSignedIn.userGroup == string(filePermissions[i].groupId) && 
                string(filePermissions[i].fileId) == currentFileId)
            {
                  for (int j = 0; j < 3; j++)
                  {
                        if (string(filePermissions[i].permissions[j]) == "WRITE")
                        {
                              canWrite = true;
                        }
                  }
            }
         
      }

      // this will edit the scores.
       if (canWrite)
      {
            cout << "\tScore \tWeight\n";
            for (int iScore = 0; iScore < scores.size(); iScore++)
            {
                  cout << "(" << iScore + 1 << ")"
                       << "\t";
                  float score = getScore(iScore);
                  float weight = getWeight(iScore);

                  cout << score << "%"
                       << "\t"
                       << weight;

                  cout << endl;
            }
      }
      else
      {
             cout << "\tScore \tWeight\n";
            for (int iScore = 0; iScore < scores.size(); iScore++)
            {
                  cout << "(" << iScore + 1 << ")"
                       << "\t";
                  cout << endl;
            }
      }
      cout << "(0)\tExit\n";
      // prompt
      bool done = false;
      while (!done)
      {
            // prompt
            int iScoreEdit;
            cout << "Which score would you like to edit (0-"
                 << scores.size()
                 << "): ";
            cin >> iScoreEdit;

            // validate score number
            while (iScoreEdit > scores.size() || iScoreEdit < 0)
            {
                  cout << "Invalid number.  Select a number between 0 and "
                       << scores.size()
                       << ": ";
                  cin >> iScoreEdit;
            }

            // from 1 based to 0 based
            iScoreEdit--;

            // edit the score
            if (iScoreEdit != -1)
            {
                  // edit score
                  editScore(iScoreEdit);

                  // continue
                  char response;
                  cout << "Do you want to edit another score? (Y/N) ";
                  cin >> response;
                  done = (response == 'N' || response == 'n');
            }
            else
                  done = true;
      }
      return;
}

/************************************************
 * Display scores
 ***********************************************/
void StudentGrade::displayScores(Ace* filePermissions, User userSignedIn)
{
      if (scores.size() == 0)
            return;

      bool canRead;

      for (int i = 0; i < sizeof(filePermissions) / sizeof(Ace); i++)
      {
            if (userSignedIn.userGroup == string(filePermissions[i].groupId) && 
                string(filePermissions[i].fileId) == currentFileId)
            {
                  for (int j = 0; j < 3; j++)
                  {
                        if (string(filePermissions[i].permissions[j]) == "READ")
                        {
                              canRead = true;
                        }
                  }
            }
         
      }

      // this will display the scores.
    if (name == userSignedIn.name || canRead || name == userSignedIn.fullName)
      {
            // name

            cout << "Student name:\n\t"
                 << name
                 << endl;

            // grade
            cout << "Grade:\n\t"
                 << getNumberGrade() << "%"
                 << " : "
                 << getLetterGrade()
                 << endl;

            // detailed score
            cout << "Detailed score:\n"
                 << "\tScore \tWeight\n";
            for (int iScore = 0; iScore < scores.size(); iScore++)
                  cout << "\t"
                       << getScore(iScore) << "%"
                       << "\t"
                       << getWeight(iScore)
                       << endl;
            // done
            return;
      }
      // done
      return;
}

/***************************************************
 * Letter Grade include A- and C+
 ***************************************************/
string StudentGrade::getLetterGrade()
{
      const char chGrades[] = "FFFFFFDCBAA";
      int nGrade = (int)getNumberGrade();

      // paranioa will destroy ya
      assert(nGrade >= 0.0 && nGrade <= 100.0);

      // Letter grade
      string s;
      s += chGrades[nGrade / 10];

      // and the + and - as necessary
      if (nGrade % 10 >= 7 && nGrade / 10 < 9 && nGrade / 10 > 5)
            s += "+";
      if (nGrade % 10 <= 2 && nGrade / 10 < 10 && nGrade / 10 > 5)
            s += "-";

      return s;
}

/***************************************************
 * Number grade guarenteed to be between 0 - 100
 ***************************************************/
float StudentGrade::getNumberGrade()
{
      // add up the scores
      float possible = 0.0;
      float earned = 0.0;
      for (int iScore = 0; iScore < scores.size(); iScore++)
      {
            earned += scores[iScore].score * scores[iScore].weight;
            possible += scores[iScore].weight;
      }

      if (possible == 0.0)
            return 0.0;
      else
            return (earned / possible);
}

/*****************************************************************
 *****************************************************************
 *                           INTERFACE                           *
 *****************************************************************
 *****************************************************************/
class Interface
{
    public:
      Interface();

      void display(Ace* filePermissions, User userSignedIn);
      void interact(Ace* filePermissions, User userSingedIn);

    private:
      int promptForStudent();
      vector<StudentGrade> students;
};

/*************************************************
 * Prompt the user for which student it to be worked
 * with.  Return -1 for none
 *************************************************/
int Interface::promptForStudent()
{
      int iSelected;

      // prompt
      cout << "Which student's grade would you like to review?\n";
      for (int iStudent = 0; iStudent < students.size(); iStudent++)
            cout << '\t'
                 << iStudent + 1
                 << ".\t"
                 << students[iStudent].getName()
                 << endl;
      cout << "\t0.\tNo student, exit\n";
      cout << "> ";

      // get input
      cin >> iSelected;
      while (iSelected < 0 || iSelected > students.size())
      {
            cout << "Invalid selection.  Please select a number between 1 and "
                 << students.size()
                 << " or select -1 to exit\n";
            cout << ">";
            cin >> iSelected;
      }

      return --iSelected;
}

/***********************************************
 * update the student records interactively
 ***********************************************/
void Interface::interact(Ace* filePermissions, User userSignedIn)
{
      int iSelected;

       if(userSignedIn.userGroup == "STUDENT")
       {
            for(int i = 0; i< students.size(); i++)
            {
                  if(userSignedIn.fullName == students[i].getName())
                  {
                        iSelected = i;
                  }
            }

          students[iSelected].displayScores(filePermissions, userSignedIn);
       } else {
         
            while (-1 != (iSelected = promptForStudent()))
            {
                  // edit grades as necessary
                  students[iSelected].editScores(filePermissions, userSignedIn);

                  // show the results
                  students[iSelected].displayScores(filePermissions, userSignedIn);

                  // visual separater
                  cout << "---------------------------------------------------\n";
            }
      }

      return;
}

/*****************************************************
 * CONSTRUCTOR
 * Populate the grades list from a file
 ****************************************************/
Interface::Interface()
{
      for (int i = 0; i < sizeof(files) / sizeof(File); i++)
      {
            StudentGrade student(files[i]);
            students.push_back(student);
      }
}

/**************************************************
 * DISPLAY
 * Display stuff
 *************************************************/
void Interface::display(Ace* filePermissions, User userSignedIn)
{
      for (int i = 0; i < students.size(); i++)
            students[i].displayScores(filePermissions, userSignedIn);
}


/**************************************************************
 * Ace
 * All the permissions currently in the system
 *************************************************************/
Ace filePermissions[] = 
{
      {"STUDENT", "1", {}},
      {"GRADER", "1", {"WRITE"}},
      {"PROFESSOR", "1", {"READ", "WRITE"}},
      {"PUBLIC", "1", {}},
      {"STUDENT", "2", {}},
      {"GRADER", "2", {"WRITE"}},
      {"PROFESSOR", "2", {"READ", "WRITE"}},
      {"PUBLIC", "2", {}},
      {"STUDENT", "3", {}},
      {"GRADER", "3", {"WRITE"}},
      {"PROFESSOR", "3", {"READ", "WRITE"}},
      {"PUBLIC", "3", {}}
};

/**************************************************************
 * USER
 * All the users currently in the system
 *************************************************************/
const User users[] =
    {
        {"Bob", "passwordBob", "GRADER"},
        {"Hans", "passwordHans", "GRADER"},
        {"Sam", "passwordSam", "STUDENT", "Samual Stevenson"},
        {"Sue", "passwordSue", "STUDENT", "Susan Bakersfield"},
        {"Sly", "passwordSly", "STUDENT", "Sylvester Stallone"}};

#define ID_INVALID -1
/**********************************************
 * authenticate the user
 *********************************************/
int authenticate(User &userSignedIn)
{
      // prompt for username
      string name;
      cout << "Username: ";
      cin >> name;

      // prompt for password
      string password;
      cout << "Password: ";
      cin >> password;

      // search for username. If found, verify password
      for (int idUser = 0; idUser < sizeof(users) / sizeof(users[0]); idUser++)
            if (name == string(users[idUser].name) &&
                password == string(users[idUser].password))
            {
                  userSignedIn.name = users[idUser].name;
                  userSignedIn.userGroup = users[idUser].userGroup;
                  userSignedIn.fullName = users[idUser].fullName;
                  return idUser;
            }

      // display error
      cout << "Failed to authenticate " << name << endl;
      return ID_INVALID;
}

/*********************************************
 * Main:
 *  open files
 *  edit scores
 *  save files
 ********************************************/
int main()
{
      User userSignedIn;
      authenticate(userSignedIn);


      Interface interface;
      interface.interact(filePermissions, userSignedIn);

      return 0;
}
