#ifndef DATAFLOWEXAMPLES_H
#define DATAFLOWEXAMPLES_H

#include <vector>
#include <thread>
#include <memory>
#include <format>

#include "dataflow/node.h"

namespace Dataflow
{
	class NumberGenNode : public BaseNode
	{
	public:
		NumberGenNode(std::weak_ptr<Bus> wpBus) :
			BaseNode(100),
			m_nextNumber(0)
		{
			m_outputs.reserve(1);
			m_outputs.emplace_back(TopicId::Topic_NumberGen);

			BaseNode::connectAndStart(wpBus);
		}
	private:
		void fire() override
		{
			Message out = Message(
				TopicId::Topic_NumberGen,
				"number_gen",
				std::format("{}", m_nextNumber));
			out.userData = m_nextNumber;
			m_outputs[0].produce(out);

			m_nextNumber++;
		}

		float m_nextNumber;
	};

	class AddNode : public BaseNode
	{
	public:
		AddNode(std::weak_ptr<Bus> wpBus) :
			BaseNode(500)
		{
			m_inputs.reserve(1);
			m_inputs.emplace_back(TopicId::Topic_NumberGen, 2);

			m_outputs.reserve(1);
			m_outputs.emplace_back(TopicId::Topic_Sum);

			BaseNode::connectAndStart(wpBus);
		}
	private :
		void fire() override 
		{ 
			Message op1 = m_inputs[0].consume();
			Message op2 = m_inputs[0].consume();

			Message sum = Message(
				TopicId::Topic_Sum,
				"add",
				std::format(" {} + {} ", op1.userData, op2.userData));

			sum.userData = op1.userData + op2.userData;

			m_outputs[0].produce(sum);
		}
	};

	class SubNode : public BaseNode
	{
	public:
		SubNode(std::weak_ptr<Bus> wpBus) :
			BaseNode(500)
		{
			m_inputs.reserve(1);
			m_inputs.emplace_back(TopicId::Topic_NumberGen, 2);

			m_outputs.reserve(1);
			m_outputs.emplace_back(TopicId::Topic_Dif);

			BaseNode::connectAndStart(wpBus);
		}
	private:
		void fire() override
		{
			Message op1 = m_inputs[0].consume();
			Message op2 = m_inputs[0].consume();

			Message sum = Message(
				TopicId::Topic_Dif,
				"subtract",
				std::format(" {} - {} ", op1.userData, op2.userData));

			sum.userData = op1.userData - op2.userData;

			m_outputs[0].produce(sum);
		}
	};

	class MultSumsNode : public BaseNode
	{
	public:
		MultSumsNode(std::weak_ptr<Bus> wpBus) :
			BaseNode(1000)
		{
			m_inputs.reserve(1);
			m_inputs.emplace_back(TopicId::Topic_Sum, 2);

			m_outputs.reserve(1);
			m_outputs.emplace_back(TopicId::Topic_MultSums);

			BaseNode::connectAndStart(wpBus);
		}
	private:
		void fire() override
		{
			Message op1 = m_inputs[0].consume();
			Message op2 = m_inputs[0].consume();

			Message sum = Message(
				TopicId::Topic_MultSums,
				"multiply sums",
				std::format("( {} ) * ( {} )", op1.payload, op2.payload ));

			sum.userData = op1.userData * op2.userData;

			m_outputs[0].produce(sum);
		}
	};

	class MultSumDiffNode : public BaseNode
	{
	public:
		MultSumDiffNode(std::weak_ptr<Bus> wpBus) :
			BaseNode(500)
		{
			m_inputs.reserve(2);
			m_inputs.emplace_back(TopicId::Topic_Sum);
			m_inputs.emplace_back(TopicId::Topic_Dif);

			m_outputs.reserve(1);
			m_outputs.emplace_back(TopicId::Topic_MultSumDif);

			BaseNode::connectAndStart(wpBus);
		}
	private:
		void fire() override
		{
			Message op1 = m_inputs[0].consume();
			Message op2 = m_inputs[1].consume();

			Message sum = Message(
				TopicId::Topic_MultSumDif,
				"multiply sums",
				std::format("( {} ) * ( {} )", op1.payload, op2.payload));

			sum.userData = op1.userData * op2.userData;

			m_outputs[0].produce(sum);
		}
	};

	class ResultNode : public BaseNode
	{
	public:
		ResultNode(std::weak_ptr<Bus> wpBus) :
			BaseNode(1000)
		{
			m_inputs.reserve(1);
			m_inputs.emplace_back(TopicId::Topic_MultSums);

			m_outputs.reserve(1);
			m_outputs.emplace_back(TopicId::Topic_FinalRes);

			BaseNode::connectAndStart(wpBus);
		}
	private:
		void fire() override
		{
			while (m_inputs[0].isReady())
			{
				Message op1 = m_inputs[0].consume();

				Message res = Message(
					TopicId::Topic_FinalRes,
					"result",
					std::format(" {} = {} ", op1.payload, op1.userData));

				res.userData = op1.userData;

				m_outputs[0].produce(res);
			}
		}
	};
}

#endif // DATAFLOWEXAMPLES_H