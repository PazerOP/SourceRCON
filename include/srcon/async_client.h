#pragma once

#include "client.h"

#ifdef _WIN32
#include <any>
#endif
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string_view>

namespace srcon
{
	class async_client
	{
	public:
		async_client();
		~async_client();

		srcon_addr get_addr() const;
		void set_addr(srcon_addr addr);

		std::string send_command(const std::string_view& command);
		std::shared_future<std::string> send_command_async(std::string command, bool reliable = true);

	private:
		struct RCONCommand
		{
			explicit RCONCommand(std::string cmd, bool reliable);

			bool operator==(const RCONCommand& other) const { return m_Command == other.m_Command; }

			std::string m_Command;
			bool m_Reliable = true;
			std::promise<std::string> m_Promise;
			std::shared_future<std::string> m_Future{ m_Promise.get_future().share() };
		};

#ifdef _WIN32
		struct ThreadLangData;
#endif

		struct ClientThreadData
		{
			std::string send_command(const std::string_view& command);

			client m_Client;
			mutable std::mutex m_ClientMutex;

			std::queue<std::shared_ptr<RCONCommand>> m_Commands;
			mutable std::mutex m_CommandsMutex;

			srcon_addr m_Address;
			mutable std::mutex m_AddressMutex;
			bool m_IsCancelled = false;

#ifdef _WIN32
			std::unique_ptr<ThreadLangData> m_SpawningThreadLanguage;
#endif
		};
		std::shared_ptr<ClientThreadData> m_ClientThreadData{ std::make_shared<ClientThreadData>() };
		std::thread m_ClientThread{ &ClientThreadFunc, m_ClientThreadData };

		static void ClientThreadFunc(std::shared_ptr<ClientThreadData> data);
	};
}
