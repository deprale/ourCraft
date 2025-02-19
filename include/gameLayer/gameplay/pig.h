#pragma once
#include <gameplay/entity.h>
#include <random>
#include <gameplay/ai.h>



struct Pig: public PhysicalEntity, public HasOrientationAndHeadTurnDirection,
	public MovementSpeedForLegsAnimations
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);

};


struct PigClient: public ClientEntity<Pig, PigClient>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);

};



struct PigServer: public ServerEntity<Pig>, public AnimalBehaviour < PigServer, PigDefaultSettings >
{
	bool update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition
		);

	void appendDataToDisk(std::ofstream &f, std::uint64_t eId);

};











