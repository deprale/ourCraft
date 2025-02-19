#include "multyPlayer/createConnection.h"
#include "enet/enet.h"
#include "multyPlayer/packet.h"
#include <iostream>
#include <multyPlayer/enetServerFunction.h>
#include <gameplay/entityManagerClient.h>
#include <multyPlayer/undoQueue.h>
#include <errorReporting.h>
#include <platformTools.h>


static ConnectionData clientData;

void submitTaskClient(Task &t)
{
	auto data = getConnectionData();

	Packet p;
	p.cid = data.cid;

	switch (t.type)
	{
	case Task::generateChunk:
	{
		p.header = headerRequestChunk;
		Packet_RequestChunk packetData = {};
		packetData.chunkPosition = {t.pos.x, t.pos.z};
		packetData.playersPositionAtRequest = t.playerPosForChunkGeneration;

		sendPacket(data.server, p, (char *)&packetData, sizeof(packetData), 1, 
			channelChunksAndBlocks);
		break;
	}
	case Task::placeBlock:
	{
		p.header = headerPlaceBlock;
		Packet_PlaceBlock packetData = {};
		packetData.blockPos = t.pos;
		packetData.blockType = t.blockType;
		packetData.eventId = t.eventId;

		sendPacket(data.server, p, (char *)&packetData, sizeof(packetData), 1, 
			channelChunksAndBlocks);
		break;
	}
	default:

	case Task::droppedItemEntity:
	{
		p.header = headerClientDroppedItem;
		Packet_ClientDroppedItem packetData = {};
		packetData.position = t.doublePos;
		packetData.blockType = t.blockType;
		packetData.count = t.blockCount;
		packetData.eventId = t.eventId;
		packetData.entityID = t.entityId;
		packetData.motionState = t.motionState;
		packetData.timer = t.timer;

		sendPacket(data.server, p, (char *)&packetData, sizeof(packetData), 1,
			channelChunksAndBlocks);
	}

	break;
	}


}

void submitTaskClient(std::vector<Task> &t)
{
	//todo can be merged into less requests.

	for (auto &i : t)
	{
		submitTaskClient(i);
	}
}

Packet formatPacket(int header)
{
	Packet p;
	p.cid = clientData.cid;
	p.header = header;
	return p;
}

ENetPeer *getServer()
{
	return clientData.server;
}


std::vector<Chunk *> getRecievedChunks()
{
	//auto ret = std::move(recievedChunks);
	auto ret = clientData.recievedChunks;
	clientData.recievedChunks.clear();
	return ret;
}

std::vector<Packet_PlaceBlocks> getRecievedBlocks()
{
	//auto ret = std::move(recievedBlocks);
	auto ret = clientData.recievedBlocks;
	clientData.recievedBlocks.clear();
	return ret;
}


ConnectionData getConnectionData()
{
	return clientData;
}


void recieveDataClient(ENetEvent &event, 
	EventCounter &validatedEvent, 
	RevisionNumber &invalidateRevision,
	glm::ivec3 playerPosition, int squareDistance,
	ClientEntityManager &entityManager,
	UndoQueue &undoQueue, std::uint64_t &yourTimer
	)
{
	Packet p;
	size_t size = 0;
	auto data = parsePacket(event, p, size);

	bool wasCompressed = 0;

	if(p.isCompressed())
	{
		//std::cout << "Decompressing\n";
		size_t newSize = {};
		auto compressedData = unCompressData(data, size, newSize);
		
		if (compressedData)
		{
			wasCompressed = true;
			data = (char*)compressedData;
			size = newSize;
		}
		else
		{
			//todo hard error request a hard reset.
			permaAssertComment(0, "decompression failed");
		}

		p.setNotCompressed();
	}

	switch(p.header)
	{
		case headerRecieveChunk:
		{

			Packet_RecieveChunk *chunkPacket = (Packet_RecieveChunk *)data;

			if (size != sizeof(Packet_RecieveChunk))
			{
				std::cout << "Size error " << size << "\n";
				break;
			}

			if (checkIfPlayerShouldGetChunk({playerPosition.x, playerPosition.z},
				{chunkPacket->chunk.x, chunkPacket->chunk.z}, squareDistance))
			{
				Chunk *c = new Chunk();
				c->data = chunkPacket->chunk;
				clientData.recievedChunks.push_back(c);
			}
			else
			{
				//std::cout << "Early rejected chunk by the client\n";
			}
			

			break;
		}

		case headerPlaceBlock:
		{
			Packet_PlaceBlocks b;
			b.blockPos = ((Packet_PlaceBlock *)data)->blockPos;
			b.blockType = ((Packet_PlaceBlock *)data)->blockType;
			clientData.recievedBlocks.push_back(b);
			break;
		}

		case headerPlaceBlocks:
		{
			for (int i = 0; i < size / sizeof(Packet_PlaceBlocks); i++)
			{
				clientData.recievedBlocks.push_back( ((Packet_PlaceBlocks*)data)[i] );

			}
			//std::cout << "Placed blocks..." << size / sizeof(Packet_PlaceBlocks) << "\n";

			break;
		}

		case headerValidateEvent:
		{
			validatedEvent = std::max(validatedEvent, ((Packet_ValidateEvent *)data)->eventId.counter);
			break;
		}

		case headerValidateEventAndChangeID:
		{
			Packet_ValidateEventAndChangeId &p = *(Packet_ValidateEventAndChangeId *)data;

			auto found = entityManager.droppedItems.find(p.oldId);

			if (found != entityManager.droppedItems.end())
			{
				auto entity = found->second;
				entityManager.droppedItems.erase(found);
				entityManager.droppedItems.insert({p.newId, entity});
			}

			validatedEvent = std::max(validatedEvent, p.eventId.counter);
			break;
		}

		case headerInValidateEvent:
		{
			invalidateRevision = std::max(invalidateRevision, ((Packet_InValidateEvent *)data)->eventId.revision);
			break;
		}

		case headerClientRecieveOtherPlayerPosition:
		{

			Packet_ClientRecieveOtherPlayerPosition *entity =
				(Packet_ClientRecieveOtherPlayerPosition *)data;

			//todo add the timer
			//if (p->timer + 16 < yourTimer)
			//{
			//	break; //drop too old packets
			//}

			if (entityManager.localPlayer.entityId == entity->eid)
			{
				//update local player
				entityManager.localPlayer.entity = entity->entity;
			}
			else
			{
				if (checkIfPlayerShouldGetEntity({playerPosition.x, playerPosition.z},
					entity->entity.position, squareDistance, 0))
				{

					auto found = entityManager.players.find(entity->eid);
					
					if (found == entityManager.players.end())
					{
						entityManager.players[entity->eid] = {};
						found = entityManager.players.find(entity->eid);
					}
					
					found->second.rubberBand
						.addToRubberBand(found->second.entity.position - entity->entity.position);

					found->second.entity = entity->entity;

					float restantTimer = computeRestantTimer(entity->timer, yourTimer);
					
					found->second.restantTime = restantTimer;

				}
				else
				{
					//dropped recieved entity
				}
				


			}

			break;
		}

		case headerClientRecieveDroppedItemUpdate:
		{

			Packet_RecieveDroppedItemUpdate *p = (Packet_RecieveDroppedItemUpdate *)data;

			if (sizeof(Packet_RecieveDroppedItemUpdate) != size) { break; } //todo logs or something
			
			//todo dont break if reliable
			if (p->timer + 16 < yourTimer)
			{
				break; //drop too old packets
			}

			float restantTimer = computeRestantTimer(p->timer, yourTimer);

			entityManager.addOrUpdateDroppedItem(p->eid, p->entity, undoQueue,
				restantTimer);

			//std::cout << restantTimer << "\n";

			break;
		}

		case headerUpdateZombie:
		{
			Packet_UpdateZombie *p = (Packet_UpdateZombie *)data;
			if (sizeof(Packet_UpdateZombie) != size) { break; }

			//todo dont break if reliable
			if (p->timer + 16 < yourTimer)
			{
				break; //drop too old packets
			}
			float restantTimer = computeRestantTimer(p->timer, yourTimer);

			entityManager.addOrUpdateZombie(p->eid, p->entity, restantTimer);

			break;
		}

		case headerUpdatePig:
		{
			Packet_UpdatePig *p = (Packet_UpdatePig *)data;
			if (sizeof(Packet_UpdatePig) != size) { break; }

			//todo dont break if reliable
			if (p->timer + 16 < yourTimer)
			{
				break; //drop too old packets
			}
			float restantTimer = computeRestantTimer(p->timer, yourTimer);

			entityManager.addOrUpdatePig(p->eid, p->entity, restantTimer);

			break;
		}

		case headerClientUpdateTimer:
		{
			Packet_ClientUpdateTimer *p = (Packet_ClientUpdateTimer *)data;
			if (sizeof(Packet_ClientUpdateTimer) != size) { break; } //todo logs or something

			yourTimer = p->timer;
			break;
		}

		case headerRemoveEntity:
		{
			if (sizeof(Packet_RemoveEntity) != size) { break; } //todo logs or something

			Packet_RemoveEntity *p = (Packet_RemoveEntity *)data;

			entityManager.removeEntity(p->EID);

			break;
		}

		//disconnect other player
		case headerDisconnectOtherPlayer:
		{
			Packet_DisconectOtherPlayer *eid = (Packet_DisconectOtherPlayer *)data;

			if (size == sizeof(Packet_DisconectOtherPlayer))
			{
				auto found = entityManager.players.find(eid->EID);

				if (found != entityManager.players.end())
				{
					entityManager.players.erase(found);
				}

			}

		}
		break;

		default:
		break;

	}

	if (wasCompressed)
	{
		delete[] data;
	}
}

//this is not multy threaded
void clientMessageLoop(EventCounter &validatedEvent, RevisionNumber &invalidateRevision
	,glm::ivec3 playerPosition, int squareDistance, ClientEntityManager &entityManager,
	UndoQueue &undoQueue, std::uint64_t &serverTimer, bool &disconnect)
{
	ENetEvent event;

	//ENetPacket *nextPacket = clientData.server->incomingDataTotal;
	for (int i = 0; i < 50; i++)
	{
		if (enet_host_service(clientData.client, &event, 0) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_RECEIVE:
				{
					
					recieveDataClient(event, validatedEvent, invalidateRevision,
						playerPosition, squareDistance, entityManager, undoQueue, serverTimer);
					
					enet_packet_destroy(event.packet);

					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				{
					disconnect = 1;
					break;
				}

			}
		}
		else
		{
			break;
		}
	}

	//int counter = 0;
	//auto nextPacket = clientData.client->dispatchQueue.sentinel.next;
	//
	//while (nextPacket)
	//{
	//	counter++;
	//	nextPacket = nextPacket->next;
	//}
	//
	//if (counter)
	//{
	//	std::cout << "Restant packets: " << counter << '\n';
	//}

}

void closeConnection()
{

	if (!clientData.conected) { return; }
	
	if (clientData.server)
	{
		enet_peer_disconnect(clientData.server, 0);

		enet_host_flush(clientData.client);
			
		ENetEvent event = {};

		while (enet_host_service(clientData.client, &event, 1000) > 0)
		{
			if (event.type == ENET_EVENT_TYPE_RECEIVE)
			{
				enet_packet_destroy(event.packet);
			}
			else if (event.type == ENET_EVENT_TYPE_RECEIVE)
			{
				break;
			}
		}

		enet_peer_reset(clientData.server);
	}

	if (clientData.client)
	enet_host_destroy(clientData.client);
	

}

bool createConnection(Packet_ReceiveCIDAndData &playerData, const char *c)
{
	if (clientData.conected) { return false; }

	clientData = ConnectionData{};

	clientData.client = enet_host_create(nullptr, 1, 1, 0, 0);

	ENetAddress adress = {};
	ENetEvent event = {};
	
	if (c && c[0] != 0)
	{
		enet_address_set_host(&adress, c);
	}
	else
	{
		enet_address_set_host(&adress, "127.0.0.1");
	}

	adress.port = 7771; //todo port stuff

	//client, adress, channels, data to send rightAway
	clientData.server = enet_host_connect(clientData.client, &adress, SERVER_CHANNELS, 0);

	if (clientData.server == nullptr)
	{
		enet_host_destroy(clientData.client);
		return false;
	}

	auto test = clientData.server;

	{

		// Set maximum throttle parameters for a specific client
		//clientData.server->packetThrottle = ENET_PEER_PACKET_THROTTLE_SCALE;
		//clientData.server->packetThrottleLimit = ENET_PEER_PACKET_THROTTLE_SCALE;
		//clientData.server->packetThrottleAcceleration = 2;
		//clientData.server->packetThrottleDeceleration = 2;
	}

	//see if we got events by server
	//client, event, ms to wait(0 means that we don't wait)
	if (enet_host_service(clientData.client, &event, 4000) > 0
		&& event.type == ENET_EVENT_TYPE_CONNECT)
	{
		std::cout << "client connected\n";
	}
	else
	{
		reportError("server timeout...");
		enet_peer_reset(clientData.server);
		enet_host_destroy(clientData.client);
		return false;
	}
	

	enet_peer_throttle_configure(clientData.server,
		ENET_PEER_PACKET_THROTTLE_INTERVAL, 6, 3);



	#pragma region handshake
	
	if (enet_host_service(clientData.client, &event, 2500) > 0
		&& event.type == ENET_EVENT_TYPE_RECEIVE)
	{
	
		{
			Packet p = {};
			size_t size;
			auto data = parsePacket(event, p, size);

			if (p.header != headerReceiveCIDAndData)
			{
				enet_peer_reset(clientData.server);
				enet_host_destroy(clientData.client);
				reportError("server sent wrong data");
				return false;
			}

			clientData.cid = p.cid;

			playerData = *(Packet_ReceiveCIDAndData *)data;

			//send player own info or sthing
			//sendPlayerData(e, true);

			std::cout << "received cid: " << clientData.cid << "\n";
			enet_packet_destroy(event.packet);
		};

		return true;
	}
	else
	{
		enet_peer_reset(clientData.server);
		enet_host_destroy(clientData.client);
		reportError("server handshake timeout");
		return 0;
	}

	#pragma endregion

	clientData.conected = true;
	return true;
}

