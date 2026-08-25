#include "GameServer.h"

#include "World.h"

namespace Swan {

void GameServer::tick(float dt)
{
	server_.tick(dt);

	broadcastTickUpdate();

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
		clientCount_ += 1;

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
	} else if (r.isQuit()) {
		info << "Client " << client.identifier << " disconnected.";
		clientCount_ -= 1;
	} else {
		info << "Received unknown message from client '" << client.identifier << ":";
		info << r.toString().flatten().cStr();
	}
}

void GameServer::broadcastTickUpdate()
{
	if (clientCount_ == 0) {
		return;
	}

	auto root = server_.builder();
	auto tick = root.initTick();

	// Ugh we will have to keep track of per-client world planes aren't we.
	// Let's worry about that later
	auto &plane = world_->currentPlane();
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

	server_.broadcast(root);
}

}
