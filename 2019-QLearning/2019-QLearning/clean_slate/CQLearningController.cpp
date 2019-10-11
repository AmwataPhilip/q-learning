/**
         (                                      
   (     )\ )                                   
 ( )\   (()/(   (    ) (        (        (  (   
 )((_)   /(_)) ))\( /( )(   (   )\  (    )\))(  
((_)_   (_))  /((_)(_)|()\  )\ |(_) )\ )((_))\  
 / _ \  | |  (_))((_)_ ((_)_(_/((_)_(_/( (()(_) 
| (_) | | |__/ -_) _` | '_| ' \)) | ' \)) _` |  
 \__\_\ |____\___\__,_|_| |_||_||_|_||_|\__, |  
                                        |___/   

Refer to Watkins, Christopher JCH, and Peter Dayan. "Q-learning." Machine learning 8. 3-4 (1992): 279-292
for a detailed discussion on Q Learning
*/
#include "CQLearningController.h"


CQLearningController::CQLearningController(HWND hwndMain):
	CDiscController(hwndMain),
	_grid_size_x(CParams::WindowWidth / CParams::iGridCellDim + 1),
	_grid_size_y(CParams::WindowHeight / CParams::iGridCellDim + 1)
{
}
/**
 The update method should allocate a Q table for each sweeper (this can
 be allocated in one shot - use an offset to store the tables one after the other)

 You can also use a boost multiarray if you wish
*/
void CQLearningController::InitializeLearningAlgorithm(void)
{
	// Create empty Q Table for each sweeper
	vector<pair<pair<double, int>, int>> qValues = { make_pair(make_pair(0.0f, 0), 0), make_pair(make_pair(0.0f, 0), 1) , make_pair(make_pair(0.0f, 0), 2) , make_pair(make_pair(0.0f, 0), 3) };
	initialisedSweepers = qSweepers(CParams::iNumSweepers, qTable(2,qValues));
}
/**
 The immediate reward function. This computes a reward upon achieving the goal state of
 collecting all the mines on the field. It may also penalize movement to encourage exploring all directions and 
 of course for hitting supermines/rocks!
*/
double CQLearningController::R(uint x,uint y, uint sweeper_no){
	float reward = -1; // Pointless movement penalty
	CDiscMinesweeper *currentSweeper = m_vecSweepers[sweeper_no]; // Pointer to current sweeper
	int currentMinesGathered = m_vecSweepers[sweeper_no]->MinesGathered(); // Number of mines collected by current sweeper


	// Checks to see if sweeper collided with an object
	bool hasCollided = currentSweeper->CheckForObject(m_vecObjects, CParams::dMineScale) >= 0 ? true: false;

	if (hasCollided) {
		// Get index of object collided with
		int collision = currentSweeper->CheckForObject(m_vecObjects, CParams::dMineScale);

		if ((x == 0) && (m_vecObjects[collision]->getType() == CDiscCollisionObject::Mine))
		{
			reward = (!(m_vecObjects[collision]->isDead())) ? 100 : (currentMinesGathered >= m_NumMines) ? 2000 : 0;
		}
		else if (((x == 2) && m_vecObjects[collision]->getType() == CDiscCollisionObject::SuperMine)) {
			reward = (!(m_vecObjects[collision]->isDead())) ? -200 : reward;
		}
		else {
			reward = (!(m_vecObjects[collision]->isDead())) ? -50 : reward;
		}
	}

	return 0;
}
/**
The update method. Main loop body of our Q Learning implementation
See: Watkins, Christopher JCH, and Peter Dayan. "Q-learning." Machine learning 8. 3-4 (1992): 279-292
*/
bool CQLearningController::Update(void)
{
	int maximumDist, minimumDist, index, action;
	SVector2D<int> sweeperPosition;

	//m_vecSweepers is the array of minesweepers
	uint cDead = std::count_if(m_vecSweepers.begin(),
							   m_vecSweepers.end(),
						       [](CDiscMinesweeper * s)->bool{
								return s->isDead();
							   });
	if (cDead == CParams::iNumSweepers){
		printf("All dead ... skipping to next iteration\n");
		m_iTicks = CParams::iNumTicks;
	}

	for (uint sw = 0; sw < CParams::iNumSweepers; ++sw){
		if (m_vecSweepers[sw]->isDead()) continue;
		
		// Find the closest object
		vector<int> closestObject = { m_vecSweepers[sw]->getClosestMine(), m_vecSweepers[sw]->getClosestSupermine()};
		sweeperPosition = m_vecSweepers[sw]->Position();
		random_device random;
		mt19937 generator(random());

		minimumDist = 99999; index = 0;
		for (int i = 0; i < closestObject.size(); ++i) {
			if (closestObject[i] < m_vecObjects.size() && !m_vecObjects[closestObject[i]]->isDead()) {
				SVector2D<int> distanceToObject = m_vecSweepers[sw]->Position() - m_vecObjects[closestObject[i]]->getPosition();
				if (minimumDist > Vec2DLength<int>(distanceToObject)) {
					minimumDist = Vec2DLength<int>(distanceToObject);
					index = i;
				}
			}
		}

		//2:::Select action with highest historic return:

		maximumDist = -99999; action = -1;
		for (int i = 0; i < 4; ++i) {
			if (initialisedSweepers[sw][index][i].first.first > maximumDist) {
				maximumDist = initialisedSweepers[sw][index][i].first.first;
				action = initialisedSweepers[sw][index][i].second;
			}
		}

		// Deal with exploration

		float epsilon = max(0.05, (((float)(CParams::iNumTicks - (m_iTicks * 2)) - 500) / CParams::iNumTicks));
		
		uniform_real_distribution<> floatDistibution(0, 1); // Float between 0 and 1
		float randomFloat = floatDistibution(generator);
		
		// If no action is set then pick a random action
		if (epsilon > randomFloat) {
			do {
				uniform_int_distribution<> integerDistribution(0, 3);
				action = integerDistribution(generator);
			} while (action == (m_vecSweepers[sw]->getRotation() + 2) % 4);
		}if (action == -1)
		{
			action = m_vecSweepers[sw]->getRotation();
		}

		m_vecSweepers[sw]->setRotation(ROTATION_DIRECTION (action));
		m_vecSweepers[sw]->Update(m_vecObjects);


		//4:::Update _Q_s_a accordingly:

		float discountFactor = 0.8;
		float learningRate = 1 / (1 + initialisedSweepers[sw][index][action].first.second);
		double calculatedReward = R(index, sweeperPosition.y, sw);
		double resultantState = discountFactor * maximumDist;
		double oldQValue = initialisedSweepers[sw][index][action].first.first;

		initialisedSweepers[sw][index][action].first.first += learningRate * calculatedReward  + (resultantState - oldQValue);

		m_vecSweepers[sw]->setRotation(ROTATION_DIRECTION((action + 2) % 4));
		m_vecSweepers[sw]->Update(m_vecObjects);
		m_vecSweepers[sw]->setRotation(ROTATION_DIRECTION (action));

	}
	
	CDiscController::Update(); //call the parent's class update. Do not delete this.
	
	return true;
}

CQLearningController::~CQLearningController(void)
{
	//TODO: dealloc stuff here if you need to	
}
