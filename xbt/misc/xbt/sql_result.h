#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <xbt/data_ref.h>
#ifdef _MSC_VER
#include <winsock2.h>
#endif
#include <mysql/mysql.h>

class Csql_row;

class Csql_result
{
public:
	typedef boost::shared_ptr<MYSQL_RES> ptr_t;

	Csql_row fetch_row() const;

	Csql_result(MYSQL_RES* h) : m_source(h, mysql_free_result)
	{
	}

	operator const void*() const
	{
		return c_rows() ? this : NULL;
	}

	int c_fields() const
	{
		return mysql_num_fields(h());
	}

	int c_rows() const
	{
		return mysql_num_rows(h());
	}

	void data_seek(int i)
	{
		mysql_data_seek(h(), i);
	}
private:
	MYSQL_RES* h() const
	{
		return m_source.get();
	}

	ptr_t m_source;
};

class Csql_row
{
public:
	Csql_row()
	{
	}

	Csql_row(MYSQL_ROW data, unsigned long* sizes, const Csql_result::ptr_t& source)
	{
		m_data = data;
		m_sizes = sizes;
		m_source = source;
	}

	operator const void*() const
	{
		return m_data;
	}

	str_ref operator[](size_t i) const
	{
		return m_data ? str_ref(m_data[i], m_sizes[i]) : str_ref();
	}
private:
	MYSQL_ROW m_data;
	unsigned long* m_sizes;
	Csql_result::ptr_t m_source;
};

inline Csql_row Csql_result::fetch_row() const
{
	MYSQL_ROW data = mysql_fetch_row(h());
	return Csql_row(data, mysql_fetch_lengths(h()), m_source);
}
