#ifndef DESIGN_PATTERNS_FACTORY_H
#define DESIGN_PATTERNS_FACTORY_H

#include <vector>
#include <iostream>

namespace DesignPatterns
{
	namespace AbstractFactory
	{
		struct Frame
		{
			size_t width;
			size_t height;
			std::vector<size_t> pixels;

			Frame(size_t w, size_t h) :
				width(w),
				height(h),
				pixels(w* h, 0)
			{
			}

			Frame(Frame&&) noexcept = default;
			Frame& operator=(Frame&&) noexcept = default; // will ensure the object is movable explicitly, but it already is without the c-tors specified
			Frame(const Frame&) = default;
			Frame& operator=(const Frame&) = default;
		};

		struct Scan
		{
			std::vector<float> ranges; // is movable, so Scan can be NRVO'd by moving during return
		};

		class ICamera
		{
		public:
			virtual void start() = 0;
			virtual void stop() = 0;
			virtual Frame getFrame() = 0;
		};

		class ILidar
		{
		public:
			virtual void start() = 0;
			virtual void stop() = 0;
			virtual Scan getScan() = 0;
		};

		class CameraSim : public ICamera
		{
		public:
			CameraSim(size_t frameW, size_t frameH) :
				m_frameSimCounter(0),
				m_frameWidth(frameW),
				m_frameHeight(frameH)
			{
			}

			void start() override
			{
				std::cout << "started camera sim! " << "\n"
					<< m_frameWidth << " x " << m_frameHeight << " px \n\n";
			}

			void stop() override
			{
				std::cout << "stopped camera sim! " << "\n\n";
			}

			Frame getFrame() override //NRVO, object will not be copied by return if possible
			{
				m_frameSimCounter++;

				std::cout << "simulated frame no. " << m_frameSimCounter << "\n\n";

				Frame frame(m_frameWidth, m_frameHeight);
				return frame;
			}

		private:
			size_t m_frameSimCounter;
			size_t m_frameWidth;
			size_t m_frameHeight;
		};

		class SensorSim : public ILidar
		{
		public:
			SensorSim(size_t scanRangeSize, size_t scanRangeValue) :
				m_sensorSimCounter(0),
				m_scanRangeSize(scanRangeSize),
				m_scanRangeValue(scanRangeValue),
				m_temp()
			{
			}

			void start() override
			{
				std::cout << "started sensor sim! \n"
					<< "scan ranges : \n"
					<< "{";
				for (size_t i = 0; i < m_scanRangeSize; i++)
				{
					std::cout << m_scanRangeValue;
				}
				std::cout
					<< "}"
					<< "\n\n";
			}

			void stop() override
			{
				std::cout << "stopped sensor sim! " << "\n\n";
			}

			Scan getScan() override
			{
				m_sensorSimCounter++;

				std::cout << "simulated scan no. " << m_sensorSimCounter << "\n\n";

				m_temp = Scan();
				return std::move(m_temp); // ensure only creating once by moving ownership on return, m_temp is valid but unspecified
			}

		private:
			size_t m_sensorSimCounter;
			size_t m_scanRangeSize;
			size_t m_scanRangeValue;
			Scan m_temp;
		};

		class CameraMock : public ICamera
		{
		public:
			void start() override
			{
				std::cout << "started mock camera! " << "\n\n";
			}

			void stop() override
			{
				std::cout << "stopped mock camera! " << "\n\n";
			}

			Frame getFrame() override
			{
				constexpr size_t width = 256;
				constexpr size_t height = 256;
				std::cout << "mock camera frame: " << "\n"
					<< width << " x " << height << " px " << "\n\n";
				return Frame{ width, height };
			}
		};

		class SensorMock : public ILidar
		{
		public:
			void start() override
			{
				std::cout << "started mock sensor! " << "\n\n";
			}

			void stop() override
			{
				std::cout << "stopped mock sensor! " << "\n\n";
			}

			Scan getScan() override //NRVO
			{
				Scan scan;
				scan.ranges = { 0.0f, 1.0f, 2.0f };

				std::cout << "mocked scan ranges: " << "\n"
					<< "{";
				for (size_t i = 0; i < scan.ranges.size(); i++)
				{
					std::cout << scan.ranges[i];
				}
				std::cout
					<< "}"
					<< "\n\n";

				return scan;
			}
		};

		class ISensorSuiteFactory
		{
		public:
			virtual ~ISensorSuiteFactory() = default;
			virtual std::unique_ptr<ICamera> createCamera() = 0;
			virtual std::unique_ptr<ILidar> createLidar() = 0;
		};

		class SimFactory : public ISensorSuiteFactory
		{
		public:
			SimFactory() = default;
			std::unique_ptr<ICamera> createCamera() override
			{
				auto camPtr = std::make_unique<CameraSim>(512, 512);
				return camPtr;
			}

			std::unique_ptr<ILidar> createLidar() override
			{
				auto lidarPtr = std::make_unique<SensorSim>(5, 3); 
				return lidarPtr; // uses the converting move constructor.
			}
		};

		class MockFactory : public ISensorSuiteFactory
		{
		public:
			MockFactory() = default;
			std::unique_ptr<ICamera> createCamera() override
			{
				auto camPtr = std::make_unique<CameraMock>();
				return camPtr;
			}

			std::unique_ptr<ILidar> createLidar() override
			{
				auto lidarPtr = std::make_unique<SensorMock>();
				return lidarPtr;
			}
		};

		void runTest()
		{
			SimFactory factory;
			//MockFactory factory;

			auto cam = factory.createCamera();						// auto
			std::unique_ptr<ILidar> lidar = factory.createLidar();  // or fully qualify, unique ptr is returned by move as is not copyable - copy elision

			cam->start();
			lidar->start();

			Frame frame = cam->getFrame();  // by value
			Scan  scan = lidar->getScan();

			cam->stop();
			lidar->stop();

			while (getchar() != '\n');
		}
	}
}

#endif // DESIGN_PATTERNS_FACTORY_H