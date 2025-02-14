#include <iostream>
#include <fstream>
#include <list>
#include <time.h>
#include <cmath>
#include <unistd.h>
#include <random>
#include <chrono>

#define MAX_STEP_LENGTH 50
#define ILLEGAL_NODE -50
#define ROOT_COORDINATE 49
#define TARGET_COL 99
#define TARGET_ROW 99
#define ITERATIONS 1000
#define SLEEP_TIME 1000
#define MAP_LENGTH 1000

using namespace std;

//when the closest Node to the sampled point is determined,
//it is made the parent Node to the point
//and the sampled Node is added to the parent Node's list of child nodes

//Node represents every Node in the rrt tree
class Node
{
public:
  //col  and row number
  int col;
  int row;
  //Node ID
  int id;
  //pointer list of all child nodes
  list<Node> child_nodes;
  //pointer to parent Node
  Node* parent;

  //public contructor
// public:
  Node(int col, int row)
  {
    this->col = col;
    this->row = row;
  }

  Node(Node* n)
  {
    this->col = n->getCol();
    this->row = n->getRow();
  }

  void setID(int id)
  {
    this->id = id;
  }

  //return the sum of square of the coordinates of the Node
  //useful when comparinfg distance to another Node
  int getSquareOfCoordinates()
  {
    return(col*col + row*row);
  }

  //returns the col
  int getCol()
  {
    return col;
  }

  //returns the row
  int getRow()
  {
    return row;
  }

  //returns the ID
  int getID()
  {
    return this->id;
  }

  //just to verify that the Node info is correct
  void display()
  {
    cout<<"Column number: "<<col<<endl;
    cout<<"Row number: "<<row<<endl;
  }

  void displayWithParentInfo()
  {
    cout<<"Column number: "<<col<<endl;
    cout<<"Row number: "<<row<<endl;
    cout<<"Parent column number: "<<parent->getCol()<<endl;
    cout<<"Parent row number: "<<parent->getRow()<<endl;
    cout<<"Child nodes: \n";
    for(list<Node>::iterator i=child_nodes.begin(); i!=child_nodes.end(); i++)
    {
      i->display();
    }
  }
};

//class Map contains information about the Map where RRT is to be implemented.
//Ex: Start, Target, Obstacles etc.
class Map
{
public:
  int starting_column;
  int starting_row;

  int target_column;
  int target_row;

  int radius;

  //structure specifies thw co-ordinates of a waypoint
  struct Waypoint
  {
    int col;
    int row;

    Waypoint(int col_num, int row_num)
    {
        col=col_num;
        row=row_num;
    }

    void display()
    {
        cout<<col<<", "<<row;
    }
  };

  //This structure specifies the four corners of an Obstacle
  struct Obstacle
  {
    int top_left_column;
    int top_left_row;
    int bottom_right_column;
    int bottom_right_row;

    Obstacle(int top_left_column, int top_left_row,
             int bottom_right_column, int bottom_right_row)
    {
      this->top_left_column=top_left_column;
      this->top_left_row = top_left_row;
      this->bottom_right_column=bottom_right_column;
      this->bottom_right_row = bottom_right_row;
    }
  };

  //A list of all waypoints (including starting and end)
  list<Waypoint> all_waypoints;

  //A list of 4 element integer arrays that specify corners of each Obstacle.
  list<Obstacle> all_obstacles;
  //An iteraot over this list tht can be used by other classes
  list<Obstacle>::iterator obs_it;

  Map()
  {
    int num_waypoints;
    int wc, wr;
    // int tlc, tlr, brc, brr;

    cout<<"Enter starting column number (0-1000, increasing rightwards): ";
    cin>>this->starting_column;
    cout<<"Enter starting row number (0-1000, increasing downwards): ";
    cin>>this->starting_row;

    cout<<"Enter target column number (0-1000, increasing rightwards): ";
    cin>>this->target_column;
    cout<<"Enter target row number (0-1000, increasing downwards): ";
    cin>>this->target_row;

    //Write obstacle data to csv file
    ofstream csv_file;
    csv_file.open("map.csv");

    //Write starting point data into csv file
    csv_file<<starting_column<<","<<starting_row<<endl;
    //Push to waypoints list
    all_waypoints.push_back(Waypoint(starting_column, starting_row));

    cout<<"Enter number of waypoints: ";
    cin>>num_waypoints;

    for(int i=0; i<num_waypoints; i++)
    {
        cout<<"Enter column number of waypoint "<<i+1<<":";
        cin>>wc;
        cout<<"Enter row number of waypoint "<<i+1<<":";
        cin>>wr;

        all_waypoints.push_back(Waypoint(wc, wr));
    }

    //Write target point data into csv file
    csv_file<<target_column<<","<<target_row<<endl;
    //Push to waypoints list
    all_waypoints.push_back(Waypoint(target_column, target_row));

    cout<<"Enter radius around waypoints: ";
    cin>>this->radius;

    //Target radius write to csv
    csv_file<<radius<<endl;

    // cout<<"Enter number of obstacles: ";
    // cin>>num_obstacles;

    // for(int i=0; i<num_obstacles; i++)
    // {
    //   cout<<"Enter obstacle top left column number: ";
    //   cin>>tlc;
    //   cout<<"Enter obstacle top left row number: ";
    //   cin>>tlr;
    //   cout<<"Enter obstacle bottom right column number: ";
    //   cin>>brc;
    //   cout<<"Enter obstacle bottom right row number: ";
    //   cin>>brr;
    //
    //   csv_file<<tlc<<","<<tlr<<","<<brc<<","<<brr<<endl;
    //
    //   //push a test Obstacle
    //   all_obstacles.push_back(*(new Obstacle(tlc, tlr, brc, brr)));
    // }

    csv_file<<100<<","<<650<<","<<300<<","<<700<<endl;
    all_obstacles.push_back(*(new Obstacle(100, 650, 300, 700)));

    csv_file<<300<<","<<700<<","<<350<<","<<900<<endl;
    all_obstacles.push_back(*(new Obstacle(300, 700, 350, 900)));

    csv_file<<650<<","<<100<<","<<700<<","<<300<<endl;
    all_obstacles.push_back(*(new Obstacle(650, 100, 700, 300)));

    csv_file<<700<<","<<300<<","<<900<<","<<350<<endl;
    all_obstacles.push_back(*(new Obstacle(700, 300, 900, 350)));

    csv_file.close();

  }

  void displayWaypoints()
  {
      //declare iterator over the Node list that contains all odes
      for(list<Waypoint>::iterator i = all_waypoints.begin(); i!=all_waypoints.end(); i++)
      {
        i->display();
        cout<<endl;
      }
  }
};

//class rrt_tree contains a list of all the nodes, and contains all the
//operations pertaining to adding nodes/sampling points etc
class rrt_tree
{
  list<Node> all_nodes;
  list<Node> path;

public:
  //add a Node to the list of all nodes
  void addNode(Node* new_node) // ✓
  {
    all_nodes.push_back(*new_node);
  }

  //returns the absolute squre of distance between two nodes
  int distanceBetween(Node node1, Node node2) // ✓
  {
    //this is literal black magic
    return sqrt(pow((node1.getCol()-node2.getCol()), 2) + pow((node1.getRow()-node2.getRow()), 2));
  }

  //tells whether a noed at given coordinates is legal or not
  bool isLegal(int new_col, int new_row, Map rrt_map, Node* q_near)
  {
    //Checking if the new proposed node on an existing vertex of the tree
    for(list<Node>::iterator i = all_nodes.begin(); i!=all_nodes.end(); i++)
    {
      if(i->getCol()==new_col&&i->getRow()==new_row)
        return false;
    }

    //checking if the proposed new node lies in an obsacle
    //How to do it: start from top left corner to bottom right, create list of
    //illegal rows and columns (br[0]-tl[0]->rows)
    for(rrt_map.obs_it=rrt_map.all_obstacles.begin();
      rrt_map.obs_it!=rrt_map.all_obstacles.end(); rrt_map.obs_it++)
    {
      if(new_col<=rrt_map.obs_it->bottom_right_column &&
        new_col>=rrt_map.obs_it->top_left_column)
        if(new_row<=rrt_map.obs_it->bottom_right_row &&
          new_row>=rrt_map.obs_it->top_left_row)
          return false;
    }

    //If the edge between q_near and q_new passes through an obstacle,
    //then q_new is still illegal. This checks that.
    //Consider a rectangle with opposite corners being q_near and q_new.
    //If any portion of the rectangle overlaps an obstacle, then node is illegal
    for(int i_col=q_near->getCol(); i_col!=new_col;
      i_col+=(new_col-q_near->getCol())/abs(new_col-q_near->getCol()))//dividing the difference with magnitude gives +-1
    {
      for(int i_row=q_near->getRow(); i_row!=new_row;
        i_row+=(new_row-q_near->getRow())/abs(new_row-q_near->getRow()))
      {
        for(rrt_map.obs_it=rrt_map.all_obstacles.begin();
          rrt_map.obs_it!=rrt_map.all_obstacles.end(); rrt_map.obs_it++)
        {
          if(i_col<=rrt_map.obs_it->bottom_right_column &&
            i_col>=rrt_map.obs_it->top_left_column)
            if(i_row<=rrt_map.obs_it->bottom_right_row &&
              i_row>=rrt_map.obs_it->top_left_row)
            {
              cout<<"Nearest before problem: \n";
              q_near->display();
              cout<<"Present coords are col, row"<<new_col<<", "<<new_row<<endl;
              return false;
            }
        }
      }
    }

    return true;
  }

  Node* generateRandomConfig() // ✓
  {
    //literal black magic
    mt19937 mt(chrono::steady_clock::now().time_since_epoch().count());
    Node* q_rand = new Node(mt()%MAP_LENGTH, mt()%MAP_LENGTH);//NOTE: must be %100
    return q_rand;
  }

  Node* nearestVertex(Node* q_rand)
  {
    Node nearest_node=*all_nodes.begin();
    for(list<Node>::iterator i = all_nodes.begin(); i!=all_nodes.end(); i++)
    {
      nearest_node=(distanceBetween(*q_rand, *i) < distanceBetween(*q_rand, nearest_node))?*i:nearest_node;
    }

    Node* q_near = new Node(nearest_node);
    return q_near;
  }

  Node* newConfiguration(Node* q_rand, Node* q_near, Map rrt_map)
  {
    Node* q_new;
    if(distanceBetween(*q_rand, *q_near)>MAX_STEP_LENGTH)
    {
      float scale_factor=(float)MAX_STEP_LENGTH/distanceBetween(*q_rand, *q_near);

      int col_diff = q_rand->getCol()-q_near->getCol();
      int row_diff = q_rand->getRow()-q_near->getRow();

      int new_col = q_near->getCol()+scale_factor*col_diff;
      int new_row = q_near->getRow()+scale_factor*row_diff;

      //RESTARTS THE Node MAKING PrOCESS IF NEW Node IS ILLEGAL
      if(!isLegal(new_col, new_row, rrt_map, q_near))
      {
        cout<<"Illegal Node. Should try again. \n";
        q_new = new Node(ILLEGAL_NODE, ILLEGAL_NODE);
      }
      else
        q_new = new Node(new_col, new_row);

      return q_new;
    }
    //if the sampled point is less than step_length units away then set
    //q_new as the sampled q_rand itself
    else
    {
      //RESTARTS THE Node MAKING PrOCESS IF NEW Node IS ILLEGAL
      if(!isLegal(q_rand->getCol(), q_rand->getRow(), rrt_map, q_near))
      {
        cout<<"Illegal Node. Should try again. \n\n";
        q_new = new Node(ILLEGAL_NODE, ILLEGAL_NODE);
      }
      else
        q_new = new Node(q_rand);

      return q_new;
    }
  }

  //Sets nearest Node as parent of new Node
  //Sets new Node as child of parent Node by pushing to list
  void addEdge(Node* q_near)
  {
    for(list<Node>::iterator i = all_nodes.begin(); i!=all_nodes.end(); i++)
    {
      if(i->getCol()==q_near->getCol()&&i->getRow()==q_near->getRow())
      {
        all_nodes.back().parent=&(*i);
        i->child_nodes.push_back(all_nodes.back());
      }
    }
  }

  //TRace parents from last node to root node
  void tracePath()
  {
    list<Node> path;
    for(Node  iterator=all_nodes.back(); iterator.parent; iterator=*(iterator.parent))
    {
      path.push_back(iterator);
    }
    this->path=path;
  }

  //to display the info of all the nodes part of the rrt_tree
  void showAllNodes()
  {
    //declare iterator over the Node list that contains all odes
    for(list<Node>::iterator i = all_nodes.begin(); i!=all_nodes.end(); i++)
    {
      i->displayWithParentInfo();
      cout<<endl;
    }
  }

  //Clears the CSV files
  void clearCSV()
  {
    ofstream csv_file;
    csv_file.open("tree.csv");
    csv_file.close();

    csv_file.open("path.csv");
    csv_file.close();
  }

  //Writes all the Node to a csv file
  //Format: col_num, row_num, parent_col_num, parent_row_num
  void writeNodeToCSV()
  {
    ofstream csv_file;
    csv_file.open("tree.csv", ios::app);
    csv_file<<all_nodes.back().getCol()<<","<<all_nodes.back().getRow()
      <<","<<all_nodes.back().parent->getCol()<<","
      <<all_nodes.back().parent->getRow()<<endl;
    csv_file.close();
  }

  //to writw the info of all the nodes in path to csv file
  void writePathToCSV()
  {
    ofstream csv_file;
    csv_file.open("path.csv", ios::app);
    //declare iterator over the Node list that contains all odes
    for(list<Node>::iterator i = --path.end(); i!=path.begin(); i--)
    {
      csv_file<<i->getCol()<<","<<i->getRow()<<endl;
    }
    csv_file<<path.begin()->getCol()<<","<<path.begin()->getRow()<<endl;
    csv_file.close();
  }
};

int main()
{
  cout<<"Hello world!\n";
  // rrt_tree main_tree;

  //Function to accept corners of rectiangular Obstacle
  Map rrt_map;

  // Node* q_null = new Node(rrt_map.starting_column, rrt_map.starting_row);
  // q_null->parent = NULL;
  //
  // Node* q_rand;
  // Node* q_near;
  // Node* q_new;
  // Node* q_root = new Node(rrt_map.starting_column, rrt_map.starting_row);
  // q_root->parent = q_null;
  //
  // main_tree.addNode(q_root);
  //
  // main_tree.clearCSV();
  // main_tree.writeNodeToCSV();

  // for(int i = 0; i>-1; i++)
  // {
  //   // usleep(SLEEP_TIME);
  //
  //   q_rand = main_tree.generateRandomConfig();
  //   cout<<"Randomly sampled point is: \n";
  //   q_rand->display();
  //
  //   q_near = main_tree.nearestVertex(q_rand);
  //   cout<<"Nearest Node is: \n";
  //   q_near->display();
  //
  //   q_new = main_tree.newConfiguration(q_rand, q_near, rrt_map);
  //
  //   //If the newConfiguration is illegal, repeat iteration of the loop
  //   //to try again
  //   if(q_new->getCol()==ILLEGAL_NODE||q_new->getRow()==ILLEGAL_NODE)
  //   {
  //     i--;
  //     continue;
  //   }
  //
  //   else
  //   {
  //     cout<<"New Node made at: \n";
  //     q_new->display();
  //
  //     main_tree.addNode(q_new);
  //
  //     main_tree.addEdge(q_near);
  //
  //     main_tree.writeNodeToCSV();
  //
  //     cout<<endl;
  //
  //     if(q_new->getCol()>=rrt_map.target_column-rrt_map.radius&&
  //       q_new->getCol()<=rrt_map.target_column+rrt_map.radius&&
  //       q_new->getRow()>=rrt_map.target_row-rrt_map.radius&&
  //       q_new->getRow()<=rrt_map.target_row+rrt_map.radius)
  //       break;
  //   }
  // }

  rrt_tree tree;
  tree.clearCSV();

  list<Map::Waypoint>::iterator prev_it=rrt_map.all_waypoints.begin();
  for(list<Map::Waypoint>::iterator curr_it=++rrt_map.all_waypoints.begin();
        curr_it!=rrt_map.all_waypoints.end(); curr_it++, prev_it++)
  {
      std::cout << "bruh" << '\n';

      rrt_tree main_tree;

      Node* q_null = new Node(prev_it->col, prev_it->row);
      q_null->parent = NULL;

      Node* q_rand;
      Node* q_near;
      Node* q_new;
      Node* q_root = new Node(prev_it->col, prev_it->row);
      q_root->parent = q_null;

      main_tree.addNode(q_root);

      main_tree.writeNodeToCSV();

      for(int i = 0; i>-1; i++)
      {
        // usleep(SLEEP_TIME);

        q_rand = main_tree.generateRandomConfig();
        cout<<"Randomly sampled point is: \n";
        q_rand->display();

        q_near = main_tree.nearestVertex(q_rand);
        cout<<"Nearest Node is: \n";
        q_near->display();

        q_new = main_tree.newConfiguration(q_rand, q_near, rrt_map);

        //If the newConfiguration is illegal, repeat iteration of the loop
        //to try again
        if(q_new->getCol()==ILLEGAL_NODE||q_new->getRow()==ILLEGAL_NODE)
        {
          i--;
          continue;
        }

        else
        {
          cout<<"New Node made at: \n";
          q_new->display();

          main_tree.addNode(q_new);

          main_tree.addEdge(q_near);

          main_tree.writeNodeToCSV();

          cout<<endl;

          if(q_new->getCol()>=curr_it->col-rrt_map.radius&&
            q_new->getCol()<=curr_it->col+rrt_map.radius&&
            q_new->getRow()>=curr_it->row-rrt_map.radius&&
            q_new->getRow()<=curr_it->row+rrt_map.radius)
            break;
        }
      }

      //Trace the path from target to root node
      main_tree.tracePath();

      //Write the path to a CSV file for visualisation
      main_tree.writePathToCSV();
  }

  // main_tree.showAllNodes();

  rrt_map.displayWaypoints();

  //Read the waypoints from csv file
  std::ifstream csv_file;
  csv_file.open("path.csv");
  std::string line;

  while(!csv_file.eof())
  {
      csv_file>>line;
      std::cout<<line<<" ";
  }

  return(0);
}
