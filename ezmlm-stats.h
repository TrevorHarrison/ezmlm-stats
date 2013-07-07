#ifndef EZMLM_STATS_H
#define EZMLM_STATS_H

#include <time.h>
#include <map>
#include <vector>

class Cezmlm_stats
{
public:
	Cezmlm_stats();
	~Cezmlm_stats();

	bool		getstats(const string &listdir, time_t starttime, time_t endtime, bool traffic, bool senders);
	void		dumpdailystats(FILE *fout);
	void		dumpsenderstats(FILE *fout);

protected:
	struct statrec
	{
		struct 
		{
			int pop;
			int adds;
			int subs;
		} subinfo[3];

		void updatesubtotals(statrec *prev)
		{ 
			for (int i = 0; i < 2; i++)
			{
				subinfo[i].pop = (prev ? prev->subinfo[i].pop : 0) + subinfo[i].adds - subinfo[i].subs;
			}
			subinfo[2].pop = subinfo[0].pop + subinfo[1].pop;
			subinfo[2].adds = subinfo[0].adds + subinfo[1].adds;
			subinfo[2].subs = subinfo[0].subs + subinfo[1].subs;
		}

		struct
		{
			int count;
		} msginfo;
	};

	int xy;
	typedef std::map< time_t, statrec > DailyStats;
	typedef std::vector< statrec > WeeklyStats, HourlyStats;
	typedef std::vector< statrec* > StatRecList;
	typedef std::map< std::string, int > SenderStats;
	typedef std::pair< std::string, int > SenderStatRec;
	typedef std::vector< SenderStatRec > SenderStatVector;
	struct SenderStatRecComp : public binary_function<SenderStatRec, SenderStatRec, bool>
	{
		bool operator()(const SenderStatRec &x, const SenderStatRec &y) { return x.second > y.second; }
	};

	void		setupfields();
	void		clearfields();
	void		sumstats(DailyStats &stats);
	void		sumstats(WeeklyStats &stats);
	bool		process_sublog(const string &fname, bool isdigest);
	bool		process_archivedir(const string &dir);
	bool		process_archives(const string &archivedir);
	void		getstatbuckets(time_t thetime, StatRecList &buckets);
	void		writestatrec(FILE *fout, statrec &rec);

	string			m_listname;
	time_t			m_start_cut;
	time_t			m_end_cut;

	DailyStats		m_dailystats;
	DailyStats		m_monthlystats;
	DailyStats		m_yearlystats;
	WeeklyStats		m_weekstats;
	HourlyStats		m_hourstats;
	SenderStats		m_senderstats;
};

#if 0
class CDateTime
{
public:
	CDateTime();
	CDateTime(const &CDateTime &rhs);
	CDateTime(time_t thetime);
	~CDateTime();

	bool	set(const string &datestr
protected:
};
#endif

#endif
