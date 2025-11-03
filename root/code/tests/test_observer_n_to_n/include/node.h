#ifndef NNOBSERVERNODE_H
#define NNOBSERVERNODE_H

#include "message.h"
#include "message_bus.h"
#include <functional>
#include <iostream>
#include <memory>

namespace NNObserver
{
	class Node
	{
	public:
		Node(std::string_view name) : m_nodeName(name) {}
		Node(std::string_view name, std::weak_ptr<Bus> wpBus) : m_nodeName(name), m_wpBus(std::move(wpBus)) {}
		virtual ~Node()  {}
		virtual void registerMessageBus(std::weak_ptr<Bus> wpBus)
		{
			m_wpBus = std::move(wpBus); 
			// is it a good idea to store the bus pointer both in the node and in the contained subs and pubs?
		}

	protected:
		std::string m_nodeName;
		std::weak_ptr<Bus> m_wpBus;
	};
}

#endif // NNOBSERVERNODE_H