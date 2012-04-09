#pragma once

#include "config.h"
#include "connection.h"
#include "epoll.h"
#include "stats.h"
#include "tcp_listen_socket.h"
#include "tracker_input.h"
#include "udp_listen_socket.h"

class Cserver
{
public:
	class peer_key_c
	{
	public:
		peer_key_c()
		{
		}

		peer_key_c(int host, int uid)
		{
			host_ = host;
#ifdef PEERS_KEY
			uid_ = uid;
#else
			(void)uid;
#endif
		}

		bool operator==(peer_key_c v) const
		{
#ifdef PEERS_KEY
			return host_ == v.host_ && uid_ == v.uid_;
#else
			return host_ == v.host_;
#endif
		}

		bool operator<(peer_key_c v) const
		{
#ifdef PEERS_KEY
			return host_ < v.host_ || host_ == v.host_ && uid_ < v.uid_;
#else
			return host_ < v.host_;
#endif
		}

		friend std::size_t hash_value(const peer_key_c& v)
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, v.host_);
#ifdef PEERS_KEY
			boost::hash_combine(seed, v.uid_);
#endif
			return seed;
		}

		int host_;
#ifdef PEERS_KEY
		int uid_;
#endif
	};

	struct t_peer
	{
		t_peer()
		{
			mtime = 0;
		}

		long long downloaded;
		long long uploaded;
		time_t mtime;
		int uid;
		short port;
		bool left;
		boost::array<char, 20> peer_id;
	};

	typedef boost::unordered_map<peer_key_c, t_peer> t_peers;

	struct t_torrent
	{
		void clean_up(time_t t, Cserver&);
		void debug(std::ostream&) const;
		std::string select_peers(const Ctracker_input&) const;

		t_torrent()
		{
			completed = 0;
			dirty = true;
			fid = 0;
			leechers = 0;
			seeders = 0;
		}

		t_peers peers;
		time_t ctime;
		int completed;
		int fid;
		int leechers;
		int seeders;
		bool dirty;
	};

	struct t_user
	{
		t_user()
		{
			can_leech = true;
			completes = 0;
			incompletes = 0;
			peers_limit = 0;
			torrent_pass_version = 0;
			torrents_limit = 0;
			wait_time = 0;
		}

		bool can_leech;
		bool marked;
		int uid;
		int completes;
		int incompletes;
		int peers_limit;
		int torrent_pass_version;
		int torrents_limit;
		int wait_time;
	};

	void test_announce();
	int test_sql();
	void accept(const Csocket&);
	t_user* find_user_by_torrent_pass(const std::string&, const std::string& info_hash);
	t_user* find_user_by_uid(int);
	void read_config();
	void write_db_torrents();
	void write_db_users();
	void read_db_torrents();
	void read_db_torrents_sql();
	void read_db_users();
	void clean_up();
	std::string insert_peer(const Ctracker_input&, bool udp, t_user*);
	std::string debug(const Ctracker_input&) const;
	std::string statistics() const;
	shared_data select_peers(const Ctracker_input&) const;
	shared_data scrape(const Ctracker_input&, t_user*);
	int run();
	static void term();
	Cserver(Cdatabase&, const std::string& table_prefix, bool use_sql, const std::string& conf_file);

	const t_torrent* torrent(const std::string& id) const
	{
		return find_ptr(m_torrents, id);
	}

	const Cconfig& config() const
	{
		return m_config;
	}

	long long secret() const
	{
		return m_secret;
	}

	Cstats& stats()
	{
		return m_stats;
	}

	time_t time() const
	{
		return m_time;
	}
private:
	typedef boost::ptr_list<Cconnection> t_connections;
	typedef std::list<Ctcp_listen_socket> t_tcp_sockets;
	typedef std::list<Cudp_listen_socket> t_udp_sockets;
	typedef boost::unordered_map<std::string, t_torrent> t_torrents;
	typedef boost::unordered_map<int, t_user> t_users;
	typedef boost::unordered_map<std::string, t_user*> t_users_torrent_passes;

	const std::string& db_name(const std::string&) const;
	static void sig_handler(int);

	Cconfig m_config;
	Cstats m_stats;
	bool m_read_users_can_leech;
	bool m_read_users_peers_limit;
	bool m_read_users_torrent_pass;
	bool m_read_users_torrents_limit;
	bool m_read_users_wait_time;
	bool m_use_sql;
	time_t m_clean_up_time;
	time_t m_read_config_time;
	time_t m_read_db_torrents_time;
	time_t m_read_db_users_time;
	time_t m_time;
	time_t m_write_db_torrents_time;
	time_t m_write_db_users_time;
	int m_fid_end;
	long long m_secret;
	t_connections m_connections;
	Cdatabase& m_database;
	Cepoll m_epoll;
	t_torrents m_torrents;
	t_users m_users;
	t_users_torrent_passes m_users_torrent_passes;
	std::string m_announce_log_buffer;
	std::string m_conf_file;
	std::string m_torrents_users_updates_buffer;
	std::string m_scrape_log_buffer;
	std::string m_table_prefix;
	std::string m_users_updates_buffer;
};
