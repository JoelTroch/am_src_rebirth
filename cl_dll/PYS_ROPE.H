//Ryokeen

#if !defined(AFX_PYS_ROPE_H__81FCB008_F820_4143_A0D2_DEBBDF239BBF__INCLUDED_)
#define AFX_PYS_ROPE_H__81FCB008_F820_4143_A0D2_DEBBDF239BBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//This Code is almost completly fomr Nehe - I just made it work in HL

#define MAX_ROPES 100

// class Mass			---> An object to represent a mass
class Mass
{
public:
	float m;									// The mass value
	vec3_t pos;								// Position in space
	vec3_t vel;								// Velocity
	vec3_t force;								// Force applied on this mass at an instance
	vec3_t oldpos;

	Mass(float m)								// Constructor
	{
		this->m = m;
		this->vel = Vector(0,0,0);
		this->pos = Vector(0,0,0);
		this->oldpos = Vector(0,0,0);
	}

	/*
	  void applyForce(Vector3D force) method is used to add external force to the mass. 
	  At an instance in time, several sources of force might affect the mass. The vector sum 
	  of these forces make up the net force applied to the mass at the instance.
	*/
	void applyForce(vec3_t force)
	{
		this->force = this->force + force;					// The external force is added to the force of the mass
	}

	/*
	  void init() method sets the force values to zero
	*/
	void init()
	{
		force = Vector(0,0,0);
	}

	/*
	  void simulate(float dt) method calculates the new velocity and new position of 
	  the mass according to change in time (dt). Here, a simulation method called
	  "The Euler Method" is used. The Euler Method is not always accurate, but it is 
	  simple. It is suitable for most of physical simulations that we know in common 
	  computer and video games.
	*/
	void simulate(float dt)
	{
		vel = vel + (force / m) * dt;				// Change in velocity is added to the velocity.
		pos = pos + vel * dt;						// Change in position is added to the position.
	}
};

// class Simulation		---> A container object for simulating masses
class Simulation
{
public:
	int numOfMasses;								// number of masses in this container
	Mass** masses;									// masses are held by pointer to pointer. (Here Mass** represents a 1 dimensional array)
	
	Simulation(int numOfMasses, float m)			// Constructor creates some masses with mass values m
	{
		this->numOfMasses = numOfMasses;
		
		masses = new Mass*[numOfMasses];			// Create an array of pointers

		for (int a = 0; a < numOfMasses; ++a)		// We will step to every pointer in the array
			masses[a] = new Mass(m);				// Create a Mass as a pointer and put it in the array
	}

	virtual void release()							// delete the masses created
	{
		for (int a = 0; a < numOfMasses; ++a)		// we will delete all of them
		{
			delete(masses[a]);
			masses[a] = NULL;
		}
			
		delete(masses);
		masses = NULL;
	}

	Mass* getMass(int index)
	{
		if (index < 0 || index >= numOfMasses)		// if the index is not in the array
			return NULL;							// then return NULL

		return masses[index];						// get the mass at the index
	}

	virtual void init()								// this method will call the init() method of every mass
	{
		for (int a = 0; a < numOfMasses; ++a)		// We will init() every mass
			masses[a]->init();						// call init() method of the mass
	}

	virtual void solve()							// no implementation because no forces are wanted in this basic container
	{
													// in advanced containers, this method will be overrided and some forces will act on masses
	}

	virtual void simulate(float dt)					// Iterate the masses by the change in time
	{
		for (int a = 0; a < numOfMasses; ++a)		// We will iterate every mass
			masses[a]->simulate(dt);				// Iterate the mass and obtain new position and new velocity
	}

	virtual void operate(float dt)					// The complete procedure of simulation
	{
		init();										// Step 1: reset forces to zero
		solve();									// Step 2: apply forces
		simulate(dt);								// Step 3: iterate the masses by the change in time
	}
	virtual void test_collision();

};

class Spring											//An object to represent a spring with inner friction binding two masses. The spring 
														//has a normal length (the length that the spring does not exert any force)
{
public:
	Mass* mass1;										//The first mass at one tip of the spring
	Mass* mass2;										//The second mass at the other tip of the spring

	float springConstant;								//A constant to represent the stiffness of the spring
	float springLength;									//The length that the spring does not exert any force
	float frictionConstant;								//A constant to be used for the inner friction of the spring

	Spring(Mass* mass1, Mass* mass2, 
		float springConstant, float springLength, float frictionConstant)		//Constructor
	{
		this->springConstant = springConstant;									//set the springConstant
		this->springLength = springLength;										//set the springLength
		this->frictionConstant = frictionConstant;								//set the frictionConstant

		this->mass1 = mass1;													//set mass1
		this->mass2 = mass2;													//set mass2
	}

	void solve(void);																	//solve() method: the method where forces can be applied
};

class RopeSimulation : public Simulation				//An object to simulate a rope interacting with a planer surface and air
{
public:
	Spring** springs;									//Springs binding the masses (there shall be [numOfMasses - 1] of them)

	vec3_t gravitation;		
	vec3_t dir,start_origin;
	float Length;
	
	float airFrictionConstant;							//a constant of air friction applied to masses
	float frictionConstant;

	RopeSimulation(										//a long long constructor with 11 parameters starts here
		int numOfMasses,								//1. the number of masses
		float m,										//2. weight of each mass
		vec3_t gravitation,							//6. gravitational acceleration
		float airFrictionConstant,						//7. air friction constant
		vec3_t dir,vec3_t start_origin,
		float Length
		) : Simulation(numOfMasses, m)					//The super class creates masses with weights m of each
	{
		this->gravitation = gravitation;
		
		this->airFrictionConstant = airFrictionConstant;
		this->start_origin = start_origin;
		this->dir = dir;
		this->Length = Length;

		//Moved this to the CPP
	}

	void release()										//release() is overriden because we have springs to delete
	{
		Simulation::release();							//Have the super class release itself

		for (int a = 0; a < numOfMasses - 1; ++a)		//to delete all springs, start a loop
		{
			delete(springs[a]);
			springs[a] = NULL;
		}
		
		delete(springs);
		springs = NULL;
	}

	void solve()										//solve() is overriden because we have forces to be applied
	{
		for (int a = 0; a < numOfMasses - 1; ++a)		//apply force of all springs
		{
			springs[a]->solve();						//Spring with index "a" should apply its force
		}

		for (a = 0; a < numOfMasses; ++a)				//Start a loop to apply forces which are common for all masses
		{
			masses[a]->applyForce(gravitation * masses[a]->m);				//The gravitational force
			
			masses[a]->applyForce(-masses[a]->vel * airFrictionConstant);	//The air friction
				
		}
	}

	void simulate(float dt)								//simulate(float dt) is overriden because we want to simulate 														//the motion of the ropeConnectionPos
	{
		Simulation::simulate(dt);						//the super class shall simulate the masses
	}
};

class pys_rope
{
public :
       vec3_t start_origin;
	   vec3_t end_origin;

       float red;
       float green;
       float blue;
	   float mass;
       float alpha;

	   float update_range;

	   bool can_collide;

	   float springconst;
	   	float airFrictionConstant;							//a constant of air friction applied to masses
	float frictionConstant;

	float sim_speed;

	   cl_entity_t *start_ent;
	   cl_entity_t *end_ent;
	   float scale;

	   bool free;
 
	   float segment_lenght;
	   float num_segments;

	   char Sprite[512];

	   RopeSimulation* ropeSimulation;
};

class GLRopeRender
{
public:

	void DrawRopes( float fltime);
	void DrawRope( pys_rope *rope,float fltime );

	void CreateRope(char *datafile,cl_entity_t *start_source,cl_entity_t *end_source);

	void StartRenderer();

	GLRopeRender();
	virtual ~GLRopeRender();

	void DrawBeam(vec3_t start,vec3_t end,float width,char *Sprite);

	pys_rope m_Ropes[MAX_ROPES];

	int num_pys_rope;

	private:

	void TrashRope( pys_rope* );

};

extern GLRopeRender gRopeRender;


#endif