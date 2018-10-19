/*
 * @CopyRight:
 * FISCO-BCOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FISCO-BCOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>
 * (c) 2016-2018 fisco-dev contributors.
 */
/**
 * @brief : Sync peer
 * @author: jimmyshi
 * @date: 2018-10-16
 */
#pragma once
#include "Common.h"
#include <libblockchain/BlockChainInterface.h>
#include <libdevcore/FixedHash.h>
#include <libdevcore/Worker.h>
#include <libethcore/Exceptions.h>
#include <libp2p/Common.h>
#include <libp2p/P2PInterface.h>
#include <libp2p/Session.h>
#include <libtxpool/TxPoolInterface.h>
#include <map>
#include <queue>
#include <vector>


namespace dev
{
namespace sync
{
class DownloadingBlockQueuePiority
{
public:
    bool operator()(
        const std::shared_ptr<dev::eth::Block>& left, const std::shared_ptr<dev::eth::Block>& right)
    {
        if (!left || !right)
            BOOST_THROW_EXCEPTION(dev::eth::InvalidBlockDownloadQueuePiorityInput());
        return left->header().number() > right->header().number();
    }
};

struct SyncStatus
{
    SyncState state = SyncState::Idle;
    int16_t protocolId;
    unsigned startBlockNumber;
    unsigned currentBlockNumber;
    unsigned highestBlockNumber;
    bool majorSyncing = false;
};

class SyncPeerStatus
{
public:
    SyncPeerStatus(
        NodeID const& _nodeId, int64_t _number, h256 const& _genesisHash, h256 const& _latestHash)
      : nodeId(_nodeId), number(_number), genesisHash(_genesisHash), latestHash(_latestHash)
    {}
    SyncPeerStatus(const NodeInfo& _info)
      : nodeId(_info.nodeId),
        number(_info.number),
        genesisHash(_info.genesisHash),
        latestHash(_info.latestHash)
    {}

public:
    NodeID nodeId;
    int64_t number;
    h256 genesisHash;
    h256 latestHash;
};

class SyncMasterStatus
{
public:
    bool hasPeer(NodeID const& _id);
    bool newSyncPeerStatus(NodeInfo const& _info);
    std::shared_ptr<SyncPeerStatus> peerData(NodeID const& _id);

    void foreachPeer(std::function<bool(std::shared_ptr<SyncPeerStatus>)> const& _f) const;

    /// Select some peers at _percent when _allow(peer)
    NodeIDs randomSelection(
        unsigned _percent, std::function<bool(std::shared_ptr<SyncPeerStatus>)> const& _allow);

public:
    h256Hash transactionsSent;

private:
    std::map<NodeID, std::shared_ptr<SyncPeerStatus>> m_peersStatus;
    std::priority_queue<std::shared_ptr<dev::eth::Block>,
        std::vector<std::shared_ptr<dev::eth::Block>>, DownloadingBlockQueuePiority>
        m_downloadingBlockQueue;
};

}  // namespace sync
}  // namespace dev