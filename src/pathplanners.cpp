#include "pathplanners.h"
#include <cmath>
#include <queue>
using namespace cv;
using namespace std;
using namespace Eigen;

double PathPlannerGrid::distance(double x1,double y1,double x2,double y2){
  return sqrt(pow(x1-x2,2) + pow(y1-y2,2));
}

//for the class PathPlannerGrid
void PathPlannerGrid::initializeLocalPreferenceMatrix(){
  //moving globally right
  aj[1][2][0].first = 0, aj[1][2][0].second = 1; 
  aj[1][2][1].first = 1, aj[1][2][1].second = 0; 
  aj[1][2][2].first = -1, aj[1][2][2].second = 0; 
  aj[1][2][3].first = 0, aj[1][2][3].second = -1; 
  //moving globally left
  aj[1][0][0].first = 0, aj[1][0][0].second = -1; 
  aj[1][0][1].first = -1, aj[1][0][1].second = 0; 
  aj[1][0][2].first = 1, aj[1][0][2].second = 0; 
  aj[1][0][3].first = 0, aj[1][0][3].second = 1; 
  //moving globally down
  aj[2][1][0].first = 1, aj[2][1][0].second = 0; 
  aj[2][1][1].first = 0, aj[2][1][1].second = -1; 
  aj[2][1][2].first = 0, aj[2][1][2].second = 1; 
  aj[2][1][3].first = -1, aj[2][1][3].second = 0; 
  //moving globally up
  aj[0][1][0].first = -1, aj[0][1][0].second = 0; 
  aj[0][1][1].first = 0, aj[0][1][1].second = 1; 
  aj[0][1][2].first = 0, aj[0][1][2].second = -1; 
  aj[0][1][3].first = 1, aj[0][1][3].second = 0; 
}

void PathPlannerGrid::gridInversion(const PathPlannerGrid &planner){//invert visitable and non visitable cells
  rcells = planner.rcells;
  ccells = planner.ccells;
  world_grid.resize(rcells);
  for(int i = 0;i<rcells;i++) world_grid[i].resize(ccells);
  for(int i = 0;i<rcells;i++)
    for(int j = 0;j<ccells;j++)
      if(planner.world_grid[i][j].steps > 0){
        world_grid[i][j].blacks = world_grid[i][j].whites = world_grid[i][j].steps = 0;
        world_grid[i][j].tot_x = planner.world_grid[i][j].tot_x;
        world_grid[i][j].tot_y = planner.world_grid[i][j].tot_y;
        world_grid[i][j].tot = planner.world_grid[i][j].tot;
      }
      else
        world_grid[i][j].steps = 1;
}

void PathPlannerGrid::addPoint(int ind,int px, int py, double x,double y){
  if(total_points+1>path_points.size()){
    path_points.resize(1+total_points);
    pixel_path_points.resize(1+total_points);
  }
  path_points[ind].x = x;
  path_points[ind].y = y;
  pixel_path_points[ind].first = px;
  pixel_path_points[ind].second = py;
  total_points++;
}

bool PathPlannerGrid::isEmpty(int r,int c){//criteria based on which to decide whether cell is empty
  if(world_grid[r][c].blacks > world_grid[r][c].whites*0.2 || r>=rcells-1 || c>=ccells-1)//more than 20 percent
    return false;
  return true;
}

bool PathPlannerGrid::pixelIsInsideTag(int x,int y,vector<AprilTags::TagDetection> &detections,int ind){
  if(ind<0)
    return false;
  for(int i = 0;i<4;i++){
    int j = (i+1)%4;
    if((x-detections[ind].p[j].first)*(detections[ind].p[j].second-detections[ind].p[i].second) - (y-detections[ind].p[j].second)*(detections[ind].p[j].first-detections[ind].p[i].first) >= 0)
      continue;
    return false;
  }
  return true;
}
//note the different use of r,c and x,y in the context of matrix and image respectively
//check for obstacles but excludes the black pixels obtained from apriltags
int PathPlannerGrid::setRobotCellCoordinates(vector<AprilTags::TagDetection> &detections){
  if(robot_id < 0){
    if(start_grid_x == start_grid_y && start_grid_x == -1){
      cout<<"can't find the robot in tags detected"<<endl;
      return -1;
    }
    else
      return 1;
  }
  start_grid_y = detections[robot_id].cxy.first/cell_size_x;
  start_grid_x = detections[robot_id].cxy.second/cell_size_y;
  return 1;
}

int PathPlannerGrid::setGoalCellCoordinates(vector<AprilTags::TagDetection> &detections){
  if(goal_id < 0){
    if(goal_grid_x == goal_grid_y && goal_grid_x == -1){
      cout<<"can't find goal in tags detected"<<endl;
      return -1;
    }
    else
      return 1;
  }
  goal_grid_y = detections[goal_id].cxy.first/cell_size_x;
  goal_grid_x = detections[goal_id].cxy.second/cell_size_y;
  return 1;
}

void PathPlannerGrid::drawGrid(Mat &image){
  int channels = image.channels();
  if(channels != 1 && channels != 3){
    cout<<"can't draw the grid on the given image"<<endl;
    return;
  }
  Vec3b col(0,0,0);
  int r = image.rows, c = image.cols;
  for(int i = 0;i<r;i += cell_size_y)
    for(int j = 0;j<c;j++)
      if(channels == 1)
        image.at<uint8_t>(i,j) = 0;
      else
        image.at<Vec3b>(i,j) = col;
  for(int i = 0;i<c;i+=cell_size_x)
    for(int j = 0;j<r;j++)
      if(channels == 1)
        image.at<uint8_t>(j,i) = 0;
      else
        image.at<Vec3b>(j,i) = col;
  for(int i = 0;i<rcells;i++)
    for(int j = 0;j<ccells;j++){
      int ax,ay;
      if(!isEmpty(i,j)) continue;
      ax = world_grid[i][j].tot_x/world_grid[i][j].tot;
      ay = world_grid[i][j].tot_y/world_grid[i][j].tot;
      circle(image, Point(ax,ay), 8, cv::Scalar(0,0,0,0), 2);
    }
}

void PathPlannerGrid::initializeGrid(int r,int c){//image rows and columns are provided
  rcells = ceil((float)r/cell_size_y);
  ccells = ceil((float)c/cell_size_x);
  world_grid.resize(rcells);
  for(int i = 0;i<rcells;i++) world_grid[i].resize(ccells);
  for(int i = 0;i<rcells;i++)
    for(int j = 0;j<ccells;j++)
      world_grid[i][j].emptyCell();
}

void PathPlannerGrid::overlayGrid(vector<AprilTags::TagDetection> &detections,Mat &grayImage){
  threshold(grayImage,grayImage,threshold_value,255,0);
  int r = grayImage.rows, c = grayImage.cols;
  initializeGrid(r,c);
  for(int i = 0;i<r;i++){
    for(int j = 0;j<c;j++){
      int gr = i/cell_size_y, gc = j/cell_size_x;
      world_grid[gr][gc].tot++;
      if(grayImage.at<uint8_t>(i,j) == 255 || pixelIsInsideTag(j+1,i+1,detections,robot_id) || pixelIsInsideTag(j+1,i+1,detections,goal_id) || pixelIsInsideTag(j+1,i+1,detections,origin_id)){
        world_grid[gr][gc].whites++;
        grayImage.at<uint8_t>(i,j) = 255;
      }
      else{
        world_grid[gr][gc].blacks++;
        grayImage.at<uint8_t>(i,j) = 0;
      }
      world_grid[gr][gc].tot_x += j+1;//pixel values are indexed from 1
      world_grid[gr][gc].tot_y += i+1;
    }
  }
}

pair<int,int> PathPlannerGrid::setParentUsingOrientation(robot_pose &ps){
  double agl = ps.omega*180/PI;
  if(agl>-45 && agl<45) return pair<int,int> (start_grid_x,start_grid_y-1);
  if(agl>45 && agl<135) return pair<int,int> (start_grid_x+1,start_grid_y);
  if(agl>135 || agl<-135) return pair<int,int> (start_grid_x,start_grid_y+1);
  if(agl<-45 && agl>-135) return pair<int,int> (start_grid_x-1,start_grid_y);
}

void PathPlannerGrid::addGridCellToPath(int r,int c,AprilInterfaceAndVideoCapture &testbed){
  cout<<"adding cell "<<r<<" "<<c<<endl;
  int ax,ay;double bx,by;
  ax = world_grid[r][c].tot_x/world_grid[r][c].tot;
  ay = world_grid[r][c].tot_y/world_grid[r][c].tot;
  testbed.pixelToWorld(ax,ay,bx,by);
  addPoint(total_points,ax,ay,bx,by);
}

bool PathPlannerGrid::isBlocked(int ngr, int ngc){
  if(ngr<0 || ngr>=rcells || ngc<0 || ngc>=ccells || !isEmpty(ngr,ngc) || world_grid[ngr][ngc].steps)
    return true;
  return false;
}

int PathPlannerGrid::getWallReference(int r,int c,int pr, int pc){
  if(pr < 0 || pc < 0)//for global preference coverage, as parent field remains unused
    return -1;
  int ngr[4],ngc[4];
  int nx = r-pr+1, ny = c-pc+1;
  for(int i = 0;i<4;i++)
    ngr[i] = r+aj[nx][ny][i].first, ngc[i] = c+aj[nx][ny][i].second;
  if(isBlocked(ngr[1],ngc[1]))//right wall due to higher priority
    return 1;
  if(isBlocked(ngr[0],ngc[0]))//front wall, turn right, left wall reference
    return 2;
  if(isBlocked(ngr[2],ngc[2]))//left wall
    return 2;
  return -1;//
}
//find shortest traversal,populate path_points
void PathPlannerGrid::findshortest(AprilInterfaceAndVideoCapture &testbed){
  if(setRobotCellCoordinates(testbed.detections)<0)
    return;
  if(setGoalCellCoordinates(testbed.detections)<0)
    return;
  queue<pair<int,int> > q;
  q.push(make_pair(start_grid_x,start_grid_y));
  world_grid[start_grid_x][start_grid_y].parent.first = rcells;//just to define parent of 1st node
  world_grid[start_grid_x][start_grid_y].parent.second = ccells;//just to define parent of 1st node
  world_grid[start_grid_x][start_grid_y].steps = 1;
  vector<pair<int,int> > aj = {{-1,0},{0,1},{1,0},{0,-1}};
  int ngr,ngc;
  pair<int,int> t;
  while(!q.empty()){
    t = q.front();q.pop();
    if(t.first == goal_grid_x && t.second == goal_grid_y)
      break;
    for(int i = 0;i<4;i++){
      ngr = t.first+aj[i].first, ngc = t.second+aj[i].second;
      if(ngr>=rcells || ngr<0 || ngc>=ccells || ngc<0 || world_grid[ngr][ngc].steps>0 || !isEmpty(ngr,ngc))
        continue;
      world_grid[ngr][ngc].parent.first = t.first;
      world_grid[ngr][ngc].parent.second = t.second;
      world_grid[ngr][ngc].steps = world_grid[t.first][t.second].steps + 1;
      q.push(make_pair(ngr,ngc));
    }
  }
  if(!( t.first == goal_grid_x && t.second == goal_grid_y )){
    cout<<"no path to reach destination"<<endl;
    total_points = -1;//dummy to prevent function recall
    return;
  }
  total_points = 0;
  int cnt = world_grid[t.first][t.second].steps;
  pixel_path_points.resize(cnt);
  path_points.resize(cnt);
  for(int i = cnt-1;!(t.first == rcells && t.second == ccells);i--){
    addGridCellToPath(t.first,t.second,testbed);
    t = world_grid[t.first][t.second].parent;
  }
}

void PathPlannerGrid::addBacktrackPointToStackAndPath(stack<pair<int,int> > &sk,vector<pair<int,int> > &incumbent_cells,int &ic_no,int ngr, int ngc,pair<int,int> &t,AprilInterfaceAndVideoCapture &testbed){
  if(ic_no){
    incumbent_cells[ic_no] = t; 
    ic_no++;
    PathPlannerGrid temp_planner;
    temp_planner.gridInversion(*this);
    temp_planner.start_grid_x = incumbent_cells[0].first;
    temp_planner.start_grid_y = incumbent_cells[0].second;
    temp_planner.goal_grid_x = incumbent_cells[ic_no-1].first;
    temp_planner.goal_grid_y = incumbent_cells[ic_no-1].second;
    cout<<"shortest path started"<<endl;
    temp_planner.findshortest(testbed);
    cout<<"shortest path ended"<<endl;
    for(int i = temp_planner.path_points.size()-1;i>=0;i--){
      addPoint(total_points,temp_planner.pixel_path_points[i].first,temp_planner.pixel_path_points[i].second,temp_planner.path_points[i].x, temp_planner.path_points[i].y);
    }
    //for(int i = 1;i<ic_no;i++){//simply revisit the previous cells till reach the branch
      //int cellrow = incumbent_cells[i].first, cellcol = incumbent_cells[i].second;
      //addGridCellToPath(cellrow,cellcol,testbed);
    //}
    ic_no = 0;//reset to zero
  }
  world_grid[ngr][ngc].wall_reference = getWallReference(t.first,t.second,world_grid[t.first][t.second].parent.first, world_grid[t.first][t.second].parent.second);
  world_grid[ngr][ngc].steps = 1;
  world_grid[ngr][ngc].parent = t;
  addGridCellToPath(ngr,ngc,testbed);
  sk.push(pair<int,int>(ngr,ngc));
}
//each function call adds only the next spiral point in the path vector, which may occur after a return phase
void PathPlannerGrid::BSACoverageIncremental(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance){

  static int first_call = 1;
  static stack<pair<int,int> > sk;//stack is needed to remember all the previous points visited and backtrack
  if(setRobotCellCoordinates(testbed.detections)<0)//set the start_grid_y, start_grid_x
    return;
  if(!first_call){
    if(!sk.empty()){
      pair<int,int> t = sk.top();
      if(t.first != start_grid_x || t.second != start_grid_y || distance(ps.x,ps.y,path_points[total_points-1].x,path_points[total_points-1].y)>reach_distance){//ensure the robot is continuing from the last point, and that no further planning occurs until the robot reaches the required point
        cout<<"the robot has not yet reached the old target"<<endl;
        return;
      }
    }
  }
  vector<pair<int,int> > incumbent_cells(rcells*ccells);
  int ic_no = 0;

  if(first_call){
    first_call = 0;
    total_points = 0;
    sk.push(pair<int,int>(start_grid_x,start_grid_y));
    world_grid[start_grid_x][start_grid_y].parent = setParentUsingOrientation(ps);
    world_grid[start_grid_x][start_grid_y].steps = 1;//visited
    addGridCellToPath(start_grid_x,start_grid_y,testbed);//add the current robot position as target point on first call, on subsequent calls the robot position would already be on the stack from the previous call assuming the function is called only when the robot has reached the next point
    return;//added the first spiral point
  }
  int ngr,ngc,wall;//neighbor row and column

  while(!sk.empty()){//when no other explorable path remains, the robot simply keeps updating the path vector with the same grid cell(path point)
    pair<int,int> t = sk.top();
    int nx = t.first-world_grid[t.first][t.second].parent.first+1;//add one to avoid negative index
    int ny = t.second-world_grid[t.first][t.second].parent.second+1;
    if((wall=world_grid[t.first][t.second].wall_reference)>=0){//if the current cell has a wall reference to consider
      ngr = t.first+aj[nx][ny][wall].first, ngc = t.second+aj[nx][ny][wall].second;
      if(!isBlocked(ngr,ngc)){
        addBacktrackPointToStackAndPath(sk,incumbent_cells,ic_no,ngr,ngc,t,testbed);
        world_grid[ngr][ngc].wall_reference = -1;//to prevent wall exchange to right wall when following left wall
        break;// a new spiral point has been added
      }
    }
    bool empty_neighbor_found = false;
    for(int i = 0;i<4;i++){
      ngr = t.first+aj[nx][ny][i].first;
      ngc = t.second+aj[nx][ny][i].second;
      if(isBlocked(ngr,ngc))
        continue;
      empty_neighbor_found = true;
      addBacktrackPointToStackAndPath(sk,incumbent_cells,ic_no,ngr,ngc,t,testbed);
      break;
    }
    if(empty_neighbor_found) break;//a new spiral point has been added
    incumbent_cells[ic_no] = t;//add the point as a possible return phase point
    ic_no++;
    sk.pop();
    if(sk.empty()) break;// the path vector would also be empty indicating end of explorable path
    pair<int,int> next_below = sk.top();
    //the lines below are obsolete(at first thought) since the shortest path is being calculated, so wall reference and parent are obsolete on already visited points
    world_grid[next_below.first][next_below.second].parent = t;
    world_grid[next_below.first][next_below.second].wall_reference = 1;//since turning 180 degrees
  }
}

void PathPlannerGrid::BSACoverage(AprilInterfaceAndVideoCapture &testbed,robot_pose &ps){
  if(setRobotCellCoordinates(testbed.detections)<0)
    return;
  vector<pair<int,int> > incumbent_cells(rcells*ccells);
  int ic_no = 0;
  stack<pair<int,int> > sk;
  sk.push(pair<int,int>(start_grid_x,start_grid_y));
  total_points = 0;
  world_grid[start_grid_x][start_grid_y].parent = setParentUsingOrientation(ps);
  world_grid[start_grid_x][start_grid_y].steps = 1;//visited
  addGridCellToPath(start_grid_x,start_grid_y,testbed);
  int ngr,ngc,wall;//neighbor row and column

  while(!sk.empty()){
    pair<int,int> t = sk.top();
    int nx = t.first-world_grid[t.first][t.second].parent.first+1;//add one to avoid negative index
    int ny = t.second-world_grid[t.first][t.second].parent.second+1;
    if((wall=world_grid[t.first][t.second].wall_reference)>=0){
      ngr = t.first+aj[nx][ny][wall].first, ngc = t.second+aj[nx][ny][wall].second;
      if(!isBlocked(ngr,ngc)){
        addBacktrackPointToStackAndPath(sk,incumbent_cells,ic_no,ngr,ngc,t,testbed);
        world_grid[ngr][ngc].wall_reference = -1;//to prevent wall exchange to right wall when following left wall
        continue;
      }
    }
    bool empty_neighbor_found = false;
    for(int i = 0;i<4;i++){
      ngr = t.first+aj[nx][ny][i].first;
      ngc = t.second+aj[nx][ny][i].second;
      if(isBlocked(ngr,ngc))
        continue;
      empty_neighbor_found = true;
      addBacktrackPointToStackAndPath(sk,incumbent_cells,ic_no,ngr,ngc,t,testbed);
      break;
    }
    if(empty_neighbor_found) continue;
    incumbent_cells[ic_no] = t;
    ic_no++;
    sk.pop();
    if(sk.empty()) break;
    pair<int,int> next_below = sk.top();
    world_grid[next_below.first][next_below.second].parent = t;
    world_grid[next_below.first][next_below.second].wall_reference = 1;//since turning 180 degrees
  }
}
void PathPlannerGrid::findCoverageLocalNeighborPreference(AprilInterfaceAndVideoCapture &testbed,robot_pose &ps){
  if(setRobotCellCoordinates(testbed.detections)<0)
    return;
  vector<pair<int,int> > incumbent_cells(rcells*ccells);
  int ic_no = 0;
  stack<pair<int,int> > sk;
  sk.push(pair<int,int>(start_grid_x,start_grid_y));
  total_points = 0;
  world_grid[start_grid_x][start_grid_y].parent = setParentUsingOrientation(ps);
  world_grid[start_grid_x][start_grid_y].steps = 1;//visited
  addGridCellToPath(start_grid_x,start_grid_y,testbed);
  int ngr,ngc;//neighbor row and column

  while(!sk.empty()){
    pair<int,int> t = sk.top();
    int nx = t.first-world_grid[t.first][t.second].parent.first+1;//add one to avoid negative index
    int ny = t.second-world_grid[t.first][t.second].parent.second+1;
    bool empty_neighbor_found = false;
    for(int i = 0;i<4;i++){
      ngr = t.first+aj[nx][ny][i].first;
      ngc = t.second+aj[nx][ny][i].second;
      if(isBlocked(ngr,ngc))
        continue;
      empty_neighbor_found = true;
      addBacktrackPointToStackAndPath(sk,incumbent_cells,ic_no,ngr,ngc,t,testbed);
      break;
    }
    if(empty_neighbor_found) continue;
    incumbent_cells[ic_no] = t;
    ic_no++;
    sk.pop();
    if(sk.empty()) break;
    pair<int,int> next_below = sk.top();
    world_grid[next_below.first][next_below.second].parent = t;
  }
}
void PathPlannerGrid::findCoverageGlobalNeighborPreference(AprilInterfaceAndVideoCapture &testbed){
  if(setRobotCellCoordinates(testbed.detections)<0)
    return;
  vector<pair<int,int> > incumbent_cells(rcells*ccells);
  int ic_no = 0;//points in above vector
  stack<pair<int,int> > sk;
  vector<pair<int,int> > aj = {{-1,0},{0,1},{0,-1},{1,0}};//adjacent cells in order of preference
  sk.push(pair<int,int>(start_grid_x,start_grid_y));
  //parent remains -1, -1
  world_grid[start_grid_x][start_grid_y].steps = 1;
  addGridCellToPath(start_grid_x,start_grid_y,testbed);
  total_points = 0;
  while(!sk.empty()){
    pair<int,int> t = sk.top();
    int ng_no = world_grid[t.first][t.second].steps;
    if(ng_no == 5){//add yourself in possible backtrack cells
      incumbent_cells[ic_no] = t;
      ic_no++;
      sk.pop();
    }
    else{
      int ngr = t.first+aj[ng_no-1].first, ngc = t.second+aj[ng_no-1].second;
      if(isBlocked(ngr,ngc)){
        world_grid[t.first][t.second].steps = ng_no+1;
        continue;
      }
      addBacktrackPointToStackAndPath(sk,incumbent_cells,ic_no,ngr,ngc,t,testbed);
      world_grid[t.first][t.second].steps = ng_no+1;
    }
  }
}
void PathPlannerGrid::drawPath(Mat &image){
  for(int i = 0;i<total_points-1;i++){
    line(image,Point(pixel_path_points[i].first,pixel_path_points[i].second),Point(pixel_path_points[i+1].first,pixel_path_points[i+1].second),Scalar(0,0,255),2);
  }
}


//for the class PathPlannerUser
void PathPlannerUser::addPoint(int px, int py, double x,double y){
  if(total_points>=path_points.size()){
    path_points.resize(100+path_points.size());//add hundred points in one go
    pixel_path_points.resize(100+pixel_path_points.size());
  }
  path_points[total_points].x = x;
  path_points[total_points].y = y;
  pixel_path_points[total_points].first = px;
  pixel_path_points[total_points].second = py;
  total_points++;
}
void PathPlannerUser::CallBackFunc(int event, int x, int y){
  static int left_clicked = 0;
  static int x_pixel_previous = -1;
  static int y_pixel_previous = -1;
  //ignore EVENT_RABUTTONDOWN, EVENT_MBUTTONDOWN 
   if(event == EVENT_LBUTTONDOWN){
      left_clicked = (left_clicked+1)%2;
      if(left_clicked){
        path_points.clear();
        pixel_path_points.clear();
        total_points = 0;
      }
      x_pixel_previous = x;
      y_pixel_previous = y;
   }
   else if(event == EVENT_MOUSEMOVE && left_clicked){
     double xd, yd;
     testbed->pixelToWorld(x,y,xd,yd);
     addPoint(x,y,xd,yd);
     x_pixel_previous = x; 
     y_pixel_previous = y;
   }
}
void PathPlannerUser::drawPath(Mat &image){
  for(int i = 0;i<total_points-1;i++){
    line(image,Point(pixel_path_points[i].first,pixel_path_points[i].second),Point(pixel_path_points[i+1].first,pixel_path_points[i+1].second),Scalar(0,0,255),2);
  }
}

