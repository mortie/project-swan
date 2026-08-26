#include "GameServer.h"

#include "World.h"

namespace Swan {

void GameServer::onTileChange(WorldPlane::ID plane, TilePos pos, Tile::ID newID)
{
	info << "Sending update @ " << pos << " to " << clients_.size() << " clients";
	if (clients_.empty()) {
		return;
	}

	auto root = server_.builder();
	auto change = root.initTileChange();
	auto p = change.initPos();
	p.setX(pos.x);
	p.setY(pos.y);
	change.setNewTile(newID);

	broadcastToPlane(plane, root);
}

void GameServer::tick(float dt)
{
	server_.tick(dt);

	// TODO: Figure out which planes a client is on
	broadcastTickUpdateForPlane(0);

	mp_proto::ClientToServer::Reader r;
	const MPServer::ClientInfo *client;
	while ((client = server_.receive(r))) {
		onMessageFromClient(*client, r);
	}
}

void GameServer::onMessageFromClient(
	const MPServer::ClientInfo &client,
	mp_proto::ClientToServer::Reader &r)
{
	if (r.hasHello()) {
		if (!r.getHello().getRequestWorld()) {
			// Nothing to do here; the client didn't request world data,
			// so we won't do the world sync
			return;
		}

		auto root = server_.builder();
		auto sync = root.initWorldSync();

		auto modIDs = sync.initModIDs(modIDs_.size());
		for (size_t i = 0; auto &id: modIDs_) {
			modIDs.set(i, id);
		}

		// Always use plane ID 0 for now
		WorldPlane::ID planeID = 0;

		auto &plane = world_->getPlane(planeID);

		auto tiles = sync.initTiles(world_->data().tiles_.size());
		for (size_t i = 0; auto &tile: world_->data().tiles_) {
			tiles.set(i++, tile.name.c_str());
		}

		sync.setCurrentPlaneIndex(planeID);
		plane.plane->serialize(sync.initCurrentPlane());
		sync.getCurrentPlane().setWorldGen(plane.worldGen);
		sync.setWorldSeed(world_->seed());

		server_.send(client.id, root);

		clients_.push_back(ConnectedClient {
			.info = client,
			.plane = planeID,
		});
	} else if (r.isQuit()) {
		info << "Client " << client.identifier << " disconnected.";

		for (size_t i = 0; i < clients_.size(); ++i) {
			if (clients_[i].info.id != client.id) {
				continue;
			}

			clients_[i] = std::move(clients_.back());
			clients_.pop_back();
			break;
		}
	} else {
		info << "Received unknown message from client '" << client.identifier << ":";
		info << r.toString().flatten().cStr();
	}
}

void GameServer::broadcastTickUpdateForPlane(WorldPlane::ID planeID)
{
	if (clients_.empty()) {
		return;
	}

	auto root = server_.builder();
	auto tick = root.initTick();

	auto &plane = *world_->getPlane(planeID).plane;
	Ctx ctx = plane.getContext();

	size_t updatedCollectionsCount = 0;
	for (auto &coll: world_->currentPlane().entities().collections()) {
		if (coll->hasUpdated()) {
			updatedCollectionsCount += 1;
		}
	}

	auto updatedCollections = tick.initUpdatedEntityCollections(updatedCollectionsCount);
	for (size_t id = 0, idx = 0; auto &coll: world_->currentPlane().entities().collections()) {
		if (!coll->hasUpdated()) {
			idx += 1;
			continue;
		}

		coll->serializeUpdates(ctx, updatedCollections[id]);
		updatedCollections[id].setIndex(idx);
		id += 1;
		idx += 1;
	}

	broadcastToPlane(plane.id_, root);
}

void GameServer::broadcastToPlane(
	WorldPlane::ID plane,
	const mp_proto::ServerToClient::Builder &root)
{
	size_t numOnCorrectPlane = 0;
	for (auto &client: clients_) {
		if (client.plane == plane) {
			numOnCorrectPlane += 1;
		}
	}

	if (numOnCorrectPlane == 0) {
		// Don't bother, we have nobody on that plane
		return;
	} else if (numOnCorrectPlane == clients_.size()) {
		// Just broadcast, everyone is on the right plane
		server_.broadcast(root);
		return;
	}

	for (auto &client: clients_) {
		// TODO: This should be optimized,
		// we're encoding for every client now
		server_.send(client.info.id, root);
	}
}

}
