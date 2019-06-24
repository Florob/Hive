/*
* Copyright (C) 2017-2019, Emilien Vallot, Christophe Calmejane and other contributors

* This file is part of Hive.

* Hive is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* Hive is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.

* You should have received a copy of the GNU Lesser General Public License
* along with Hive.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "node.hpp"
#include "toolkit/helper.hpp"

namespace connectionMatrix
{
Node::Type Node::type() const
{
	return _type;
}

bool Node::isEntityNode() const
{
	return _type == Type::Entity;
}

bool Node::isRedundantNode() const
{
	switch (_type)
	{
		case Type::RedundantOutput:
		case Type::RedundantInput:
			return true;
		default:
			return false;
	}
}

bool Node::isRedundantStreamNode() const
{
	switch (_type)
	{
		case Type::RedundantOutputStream:
		case Type::RedundantInputStream:
			return true;
		default:
			return false;
	}
}

bool Node::isStreamNode() const
{
	switch (_type)
	{
		case Type::OutputStream:
		case Type::InputStream:
		case Type::RedundantOutputStream:
		case Type::RedundantInputStream:
			return true;
		default:
			return false;
	}
}

bool Node::isChannelNode() const
{
	switch (_type)
	{
		case Type::OutputChannel:
		case Type::InputChannel:
			return true;
		default:
			return false;
	}
}

la::avdecc::UniqueIdentifier const& Node::entityID() const
{
	return _entityID;
}

Node* Node::parent() const
{
	return const_cast<Node*>(_parent);
}

bool Node::hasParent() const
{
	return _parent != nullptr;
}

QString const& Node::name() const
{
	return _name;
}

int Node::index() const
{
	return _parent ? _parent->indexOf(const_cast<Node*>(this)) : 0;
}

int Node::indexOf(Node* child) const
{
	auto const predicate = [child](auto const& item)
	{
		return item.get() == child;
	};
	auto const it = std::find_if(std::begin(_children), std::end(_children), predicate);
	if (!AVDECC_ASSERT_WITH_RET(it != std::end(_children), "not found"))
	{
		return -1;
	}
	auto const index = std::distance(std::begin(_children), it);
	return static_cast<int>(index);
}

Node* Node::childAt(int index)
{
	if (index < 0 || index >= childrenCount())
	{
		return nullptr;
	}

	return _children.at(index).get();
}

Node const* Node::childAt(int index) const
{
	if (index < 0 || index >= childrenCount())
	{
		return nullptr;
	}

	return _children.at(index).get();
}

int Node::childrenCount() const
{
	return static_cast<int>(_children.size());
}

Node::Node(Type const type, la::avdecc::UniqueIdentifier const& entityID, Node* parent)
	: _type{ type }
	, _entityID{ entityID }
	, _parent{ parent }
	, _name{ avdecc::helper::uniqueIdentifierToString(entityID) }
{
	if (_parent)
	{
		_parent->_children.emplace_back(this);
	}
}

void Node::setName(QString const& name)
{
	_name = name;
}

EntityNode* EntityNode::create(la::avdecc::UniqueIdentifier const& entityID, bool const isMilan)
{
	return new EntityNode{ entityID, isMilan };
}

void EntityNode::accept(la::avdecc::entity::model::AvbInterfaceIndex const avbInterfaceIndex, AvbInterfaceIndexVisitor const& visitor) const
{
	Node::accept(
		[&avbInterfaceIndex, &visitor](Node* node)
		{
			if (node->isStreamNode())
			{
				auto* streamNode = static_cast<StreamNode*>(node);
				if (streamNode->avbInterfaceIndex() == avbInterfaceIndex)
				{
					visitor(streamNode);
				}
			}
		});
}

EntityNode::EntityNode(la::avdecc::UniqueIdentifier const& entityID, bool const isMilan)
	: Node{ Type::Entity, entityID, nullptr }
	, _isMilan{ isMilan }
{
}

bool EntityNode::isMilan() const noexcept
{
	return _isMilan;
}

RedundantNode* RedundantNode::createOutputNode(EntityNode& parent, la::avdecc::controller::model::VirtualIndex const redundantIndex)
{
	return new RedundantNode{ Type::RedundantOutput, parent, redundantIndex };
}

RedundantNode* RedundantNode::createInputNode(EntityNode& parent, la::avdecc::controller::model::VirtualIndex const redundantIndex)
{
	return new RedundantNode{ Type::RedundantInput, parent, redundantIndex };
}

la::avdecc::controller::model::VirtualIndex const& RedundantNode::redundantIndex() const
{
	return _redundantIndex;
}

RedundantNode::RedundantNode(Type const type, EntityNode& parent, la::avdecc::controller::model::VirtualIndex const redundantIndex)
	: Node{ type, parent.entityID(), &parent }
	, _redundantIndex{ redundantIndex }
{
}

StreamNode* StreamNode::createRedundantOutputNode(RedundantNode& parent, la::avdecc::entity::model::StreamIndex const streamIndex, la::avdecc::entity::model::AvbInterfaceIndex const avbInterfaceIndex)
{
	return new StreamNode{ Type::RedundantOutputStream, parent, streamIndex, avbInterfaceIndex };
}

StreamNode* StreamNode::createRedundantInputNode(RedundantNode& parent, la::avdecc::entity::model::StreamIndex const streamIndex, la::avdecc::entity::model::AvbInterfaceIndex const avbInterfaceIndex)
{
	return new StreamNode{ Type::RedundantInputStream, parent, streamIndex, avbInterfaceIndex };
}

StreamNode* StreamNode::createOutputNode(EntityNode& parent, la::avdecc::entity::model::StreamIndex const streamIndex, la::avdecc::entity::model::AvbInterfaceIndex const avbInterfaceIndex)
{
	return new StreamNode{ Type::OutputStream, parent, streamIndex, avbInterfaceIndex };
}

StreamNode* StreamNode::createInputNode(EntityNode& parent, la::avdecc::entity::model::StreamIndex const streamIndex, la::avdecc::entity::model::AvbInterfaceIndex const avbInterfaceIndex)
{
	return new StreamNode{ Type::InputStream, parent, streamIndex, avbInterfaceIndex };
}

la::avdecc::entity::model::StreamIndex const& StreamNode::streamIndex() const
{
	return _streamIndex;
}

la::avdecc::entity::model::AvbInterfaceIndex const& StreamNode::avbInterfaceIndex() const
{
	return _avbInterfaceIndex;
}

la::avdecc::entity::model::StreamFormat const& StreamNode::streamFormat() const
{
	return _streamFormat;
}

la::avdecc::UniqueIdentifier const& StreamNode::grandMasterID() const
{
	return _grandMasterID;
}

std::uint8_t const& StreamNode::grandMasterDomain() const
{
	return _grandMasterDomain;
}

la::avdecc::controller::ControlledEntity::InterfaceLinkStatus const& StreamNode::interfaceLinkStatus() const
{
	return _interfaceLinkStatus;
}

bool StreamNode::isRunning() const
{
	return _isRunning;
}

la::avdecc::entity::model::StreamConnectionState const& StreamNode::streamConnectionState() const
{
	return _streamConnectionState;
}

StreamNode::StreamNode(Type const type, Node& parent, la::avdecc::entity::model::StreamIndex const streamIndex, la::avdecc::entity::model::AvbInterfaceIndex const avbInterfaceIndex)
	: Node{ type, parent.entityID(), &parent }
	, _streamIndex{ streamIndex }
	, _avbInterfaceIndex{ avbInterfaceIndex }
{
}

void StreamNode::setStreamFormat(la::avdecc::entity::model::StreamFormat const streamFormat)
{
	_streamFormat = streamFormat;
}

void StreamNode::setGrandMasterID(la::avdecc::UniqueIdentifier const grandMasterID)
{
	_grandMasterID = grandMasterID;
}

void StreamNode::setGrandMasterDomain(std::uint8_t const grandMasterDomain)
{
	_grandMasterDomain = grandMasterDomain;
}

void StreamNode::setInterfaceLinkStatus(la::avdecc::controller::ControlledEntity::InterfaceLinkStatus const interfaceLinkStatus)
{
	_interfaceLinkStatus = interfaceLinkStatus;
}

void StreamNode::setRunning(bool isRunning)
{
	_isRunning = isRunning;
}

void StreamNode::setStreamConnectionState(la::avdecc::entity::model::StreamConnectionState const& streamConnectionState)
{
	_streamConnectionState = streamConnectionState;
}

ChannelNode* ChannelNode::createOutputNode(EntityNode& parent, avdecc::ChannelIdentification const& channelIdentification)
{
	return new ChannelNode{ Type::OutputChannel, parent, channelIdentification };
}

ChannelNode* ChannelNode::createInputNode(EntityNode& parent, avdecc::ChannelIdentification const& channelIdentification)
{
	return new ChannelNode{ Type::InputChannel, parent, channelIdentification };
}

avdecc::ChannelIdentification const& ChannelNode::channelIdentification() const
{
	return _channelIdentification;
}

la::avdecc::entity::model::ClusterIndex ChannelNode::clusterIndex() const
{
	return *_channelIdentification.clusterIndex;
}

std::uint16_t ChannelNode::channelIndex() const
{
	return *_channelIdentification.clusterChannel;
}

ChannelNode::ChannelNode(Type const type, Node& parent, avdecc::ChannelIdentification const& channelIdentification)
	: Node{ type, parent.entityID(), &parent }
	, _channelIdentification{ channelIdentification }
{
}

/* ************************************************************ */
/* Node Policies                                                */
/* ************************************************************ */
bool Node::CompleteHierarchyPolicy::shouldVisit(Node const* const) noexcept
{
	return true;
}

bool Node::StreamHierarchyPolicy::shouldVisit(Node const* const node) noexcept
{
	return node->isEntityNode() || node->isRedundantNode() || node->isStreamNode();
}

bool Node::StreamPolicy::shouldVisit(Node const* const node) noexcept
{
	return node->isStreamNode();
}

bool Node::ChannelHierarchyPolicy::shouldVisit(Node const* const node) noexcept
{
	if (node->isEntityNode())
	{
		auto const* const entityNode = static_cast<EntityNode const*>(node);
		// Only accept Milan Entities
		return entityNode->isMilan();
	}
	else
	{
		return node->isChannelNode();
	}
}

bool Node::ChannelPolicy::shouldVisit(Node const* const node) noexcept
{
	return node->isChannelNode();
}

} // namespace connectionMatrix
