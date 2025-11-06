#ifndef NNOBSERVERNODE_H
#define NNOBSERVERNODE_H

#include "message.h"
#include "message_bus.h"
#include <functional>
#include <iostream>
#include <memory>

namespace NNObserver
{
	class Component
	{
	public:
		Component(std::string_view name) : m_componentName(name) {}
		Component(std::string_view name, std::weak_ptr<Bus> wpBus) : m_componentName(name), m_wpBus(std::move(wpBus)) {}
		virtual ~Component()  {}
		virtual void registerMessageBus(std::weak_ptr<Bus> wpBus)
		{
			m_wpBus = std::move(wpBus); 
		}

	protected:
		std::string m_componentName;
		std::weak_ptr<Bus> m_wpBus;
	};
}

#endif // NNOBSERVERNODE_H