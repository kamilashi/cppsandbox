#ifndef NNOBSERVERNODE_H
#define NNOBSERVERNODE_H

#include "message.h"
#include "message_bus.h"
#include <functional>
#include <iostream>
#include <memory>

#define MAKE_CALLBACK(x) [this](const NNObserver::Message& m) { x(m); }

namespace NNObserver
{
	class Node
	{
	public:
		Node() : m_linkId(-1), m_wpBus{} {}
		Node(std::weak_ptr<Bus> wpBus) : m_linkId(-1), m_wpBus(std::move(wpBus)) {}
		virtual ~Node() 
		{
			auto sharedBus = m_wpBus.lock();
			if (m_linkId > -1 && sharedBus)
			{
				sharedBus->unsubscribe(m_linkId);

				std::cout << "unsubbed! \n";
			}
		}

	protected:
		int m_linkId;
		std::weak_ptr<Bus> m_wpBus;
	};
}

#endif // NNOBSERVERNODE_H