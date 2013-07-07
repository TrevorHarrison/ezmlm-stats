#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <string>
#include <algorithm>


#include "ezmlm-stats.h"

using std::string;

void Cezmlm_stats::setupfields()
{
	m_weekstats.resize(7);
	m_hourstats.resize(24);
	m_start_cut = 0;
	m_end_cut = 0;
}

void Cezmlm_stats::clearfields()
{
}


Cezmlm_stats::Cezmlm_stats()
{
	setupfields();
}

Cezmlm_stats::~Cezmlm_stats()
{
	clearfields();
}

string get1stline(const string &fname)
{
	string result;
	FILE *fin = fopen(fname.c_str(), "rt");
	if ( fin )
	{
		char temp[100];
		if ( fgets(temp, sizeof(temp), fin) )
		{
			int slen = strlen(temp);
			if ( slen && temp[slen-1] == '\n' ) temp[slen-1] = '\0';
			result = temp;
		}
		fclose(fin);
	}
	return result;
}


bool Cezmlm_stats::getstats(const string &listdir, time_t starttime, time_t endtime, bool traffic, bool senders)
{
	m_start_cut = starttime;
	m_end_cut = endtime;
	if ( m_end_cut == 0 ) m_end_cut = 0x7fffffff;

	m_listname = get1stline(listdir+string("/text/listname"));

	if ( !process_sublog(listdir+string("/Log"), false) ) return false;
	if ( !process_sublog(listdir+string("/digest/Log"), true) ) return false;
	if ( !process_archives(listdir+string("/archive")) ) return false;

	sumstats(m_dailystats);
	sumstats(m_monthlystats);
	sumstats(m_yearlystats);
	sumstats(m_weekstats);
	sumstats(m_hourstats);

	return true;
}

void Cezmlm_stats::writestatrec(FILE *fout, statrec &rec)
{
	fprintf(fout, "%d %d %d %d %d %d %d %d %d %d\n", rec.subinfo[2].pop, rec.subinfo[2].adds, rec.subinfo[2].subs, rec.subinfo[0].pop, rec.subinfo[0].adds, rec.subinfo[0].subs, rec.subinfo[1].pop, rec.subinfo[1].adds, rec.subinfo[1].subs, rec.msginfo.count);
}

#define SEC_PER_DAY (60*60*24)


void Cezmlm_stats::dumpdailystats(FILE *fout)
{
	//printf("Date,FullPop,FullAdds,FullSubs,Pop,Adds,Subs,DigestPop,DigestAdds,DigestSubs\n");
	//fprintf(fout, "#date pop adds subs regpop regadds regsubs digestpop digestadds digestsubs msgcount\n");
	fprintf(fout, 
				"#\n#ezmlm-stats daily stat dump, gnuplot formatted\n#\n"
				"##COL_DATE=1\n" 
				"##COL_POP=2\n"
				"##COL_ADDS=3\n"
				"##COL_SUBS=4\n"
				"##COL_REG_POP=5\n"
				"##COL_REG_ADDS=6\n"
				"##COL_REG_SUBS=7\n"
				"##COL_DIG_POP=8\n"
				"##COL_DIG_ADDS=9\n"
				"##COL_DIG_SUBS=10\n"
				"##COL_MSGCOUNT=11\n"
				"#\n");

	int curgroup = 0;

	fprintf(fout, "##GROUP_DAILY_START=%d\n", curgroup);

	tm lastdate, thedate;
	time_t starttime=0, thetime, endtime;
	int daycount = 0;
	for(DailyStats::iterator it = m_dailystats.begin(); it != m_dailystats.end(); it++)
	{
		thetime = (*it).first;
		//printf("time<start?: %d < %d\n", thetime, m_start_cut); 
		if ( thetime < m_start_cut ) { continue; }
		if ( thetime > m_end_cut ) break;

		daycount++;

		statrec &rec = (*it).second;
		tm thedate = *localtime(&thetime);

		if ( (thedate.tm_year*100+thedate.tm_mon) != (lastdate.tm_year*100+lastdate.tm_mon) && starttime != 0 )
		{
			fprintf(fout, "\n");
			curgroup++;
		}
		if ( !starttime ) starttime = thetime;

		fprintf(fout, "%d/%d/%d ", thedate.tm_mon+1, thedate.tm_mday, thedate.tm_year+1900);
		writestatrec(fout, rec);

		lastdate = thedate;
	}
	endtime = thetime;

	if ( daycount > 0 )
	{
		thedate = *localtime(&starttime);
		lastdate = *localtime(&endtime);
		fprintf(fout,	"##GROUP_DAILY_END=%d\n"
						"##GROUP_DAILY_STARTDATE=\"%d/%d/%d\"\n"
						"##GROUP_DAILY_ENDDATE=\"%d/%d/%d\"\n",
						curgroup, thedate.tm_mon+1, thedate.tm_mday, thedate.tm_year+1900, lastdate.tm_mon+1, lastdate.tm_mday, lastdate.tm_year+1900);
	}

	fprintf(fout, "\n");
	// this needs adjusting for localtime hours maybe, maybe not
	for(int i = 0; i < m_hourstats.size(); i++)
	{
		fprintf(fout, "%d ", i);
		writestatrec(fout, m_hourstats[i]);
	}
	curgroup++;
	fprintf(fout,	"##GROUP_HOUR_START=%d\n"
					"##GROUP_HOUR_END=%d\n\n", curgroup, curgroup);

	for(int i = 0; i < m_weekstats.size(); i++)
	{
		fprintf(fout, "%d ", i);
		writestatrec(fout, m_weekstats[i]);
	}
	curgroup++;
	fprintf(fout,	"##GROUP_WEEK_START=%d\n"
					"##GROUP_WEEK_END=%d\n\n", curgroup, curgroup);

}

void Cezmlm_stats::dumpsenderstats(FILE *fout)
{
	SenderStatVector sendervector;
	for(SenderStats::iterator it = m_senderstats.begin(); it != m_senderstats.end(); it++)
	{
		sendervector.push_back( (*it) );
	}
	sort(sendervector.begin(), sendervector.end(), SenderStatRecComp());
	for(int i = 0; i < sendervector.size(); i++)
	{
		string sender = sendervector[i].first;
		int count = sendervector[i].second;

		fprintf(fout, "%d %s\n", count, sender.c_str());
	}
}

bool Cezmlm_stats::process_sublog(const string &fname, bool isdigest)
{
	FILE *f = fopen(fname.c_str(), "rt");
	if ( !f ) return false;

	char buffer[1024];
	time_t prevtime=0;
	int index = isdigest ? 1 : 0;

	while ( fgets(buffer, sizeof(buffer), f) )
	{
		time_t thetime;
		char action;
		int fieldsread = sscanf(buffer, "%d %c", &thetime, &action);
		if ( fieldsread != 2 ) continue;
		int inc = (action == '+') ? 1 : -1;

		StatRecList buckets;
		getstatbuckets(thetime, buckets);
		for(int i = 0; i < buckets.size(); i++)
		{
			if ( inc > 0 ) buckets[i]->subinfo[index].adds++; else buckets[i]->subinfo[index].subs++;
		}
	}

	fclose(f);
	return true;
}

char *monthnames[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec", 0 };
int monthname2index(const char *s)
{
	for(int i = 0; monthnames[i]; i++)
	{
		if ( strcasecmp(s, monthnames[i]) == 0 ) return i;
	}
	return -1;
}

time_t datestr2time(const char *s)
{
	tm lt; memset(&lt, 0, sizeof(lt));
	int offset;
	char monthstr[4];
	int fieldcount = sscanf(s, "%d %3s %d %d:%d:%d %d", &lt.tm_mday, &monthstr, &lt.tm_year, &lt.tm_hour, &lt.tm_min, &lt.tm_sec, &offset);
	if ( fieldcount != 7 ) return -1;
	if ( (lt.tm_mon = monthname2index(monthstr)) == -1 ) return -1;
	lt.tm_year -= 1900;
	time_t result = timegm(&lt);
	result -= 3600 * (offset/100);
	return result;
}

void fixsender(char *s)
{
	for(int i = 0; s[i]; i++)
	{
		if ( !isalnum(s[i]) && !(s[i] == '@' || s[i] == '.') ) s[i] = ' ';
	}
}

bool Cezmlm_stats::process_archivedir(const string &dir)
{
	string fname=dir;
	fname += "/index";

	FILE *fin = fopen(fname.c_str(), "rt");
	if ( !fin ) return false;

	char buffer[1024];
	while ( fgets(buffer, sizeof(buffer), fin) && fgets(buffer, sizeof(buffer), fin) )
	{
		time_t thetime = datestr2time(buffer);
		if ( thetime < m_start_cut || thetime > m_end_cut ) continue;

		tm gmt = *gmtime(&thetime);

		StatRecList buckets;
		getstatbuckets(thetime, buckets);
		for(int i = 0; i < buckets.size(); i++)
		{
			buckets[i]->msginfo.count++;
		}

		char sender[1024];
		sscanf(buffer, "%*d %*3s %*d %*d:%*d:%*d %*d;%*s %1024[^\n]", sender);
		fixsender(sender);
		m_senderstats[string(sender)]++;
	}

	fclose(fin);
	return true;
}

#define MAX_ARCHIVE_DIR 10000
bool Cezmlm_stats::process_archives(const string &archivedir)
{
	for(int i = 0; i < MAX_ARCHIVE_DIR; i++)
	{
		char temp[10];
		sprintf(temp, "/%d", i);

		string subdir = archivedir;
		subdir += temp;

		struct stat statbuf;
		if ( stat(subdir.c_str(), &statbuf) != 0 || !S_ISDIR(statbuf.st_mode) ) break;

		if ( !process_archivedir(subdir) ) { /* nada */ }
	}

	return true;
}

void Cezmlm_stats::getstatbuckets(time_t thetime, StatRecList &buckets)
{
	buckets.clear();

	tm gmt = *localtime(&thetime);

	if ( (m_start_cut <= thetime) && (thetime <= m_end_cut) )
	{
		buckets.push_back( &m_weekstats[gmt.tm_wday] );
		buckets.push_back( &m_hourstats[gmt.tm_hour] );
	}

	// round the time off to the day
	gmt.tm_sec = 0; gmt.tm_min = 0; gmt.tm_hour = 0;
	thetime = mktime(&gmt);
	buckets.push_back( &m_dailystats[thetime] );

	// round the time off to the month
	gmt.tm_mday = 1;
	thetime = mktime(&gmt);
	buckets.push_back( &m_monthlystats[thetime] );

	// round the time off to the year
	gmt.tm_mon = 0;
	thetime = mktime(&gmt);
	buckets.push_back( &m_yearlystats[thetime] );
}

void Cezmlm_stats::sumstats(DailyStats &stats)
{
	statrec *prev = NULL;
	for(DailyStats::iterator it = stats.begin(); it != stats.end(); it++)
	{
		statrec &dstat = (*it).second;
		dstat.updatesubtotals(prev);
		prev = &dstat;
	}
}

void Cezmlm_stats::sumstats(WeeklyStats &stats)
{
	statrec total;
	memset(&total, 0, sizeof(total));

	int ssize = stats.size();
	for(int i = 0; i < ssize; i++)
	{
		stats[i].updatesubtotals(NULL);
		for(int j = 0; j < 3; j++)
		{
			total.subinfo[j].pop += stats[i].subinfo[j].pop;
			total.subinfo[j].adds += stats[i].subinfo[j].adds;
			total.subinfo[j].subs += stats[i].subinfo[j].subs;
		}
		total.msginfo.count += stats[i].msginfo.count;
	}
#if 0
	for(int i = 0; i < 3; i++)
	{
		if ( total.subinfo[i].pop == 0 ) total.subinfo[i].pop = 1;
		if ( total.subinfo[i].adds == 0 ) total.subinfo[i].adds = 1;
		if ( total.subinfo[i].subs == 0 ) total.subinfo[i].subs = 1;
	}
	if ( total.msginfo.count == 0 ) total.msginfo.count = 1;

	//printf("total pop %d\n", total.subinfo[2].pop);
	for(int i = 0; i < ssize; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			stats[i].subinfo[j].pop = (stats[i].subinfo[j].pop*100) / total.subinfo[j].pop;
			stats[i].subinfo[j].adds = (stats[i].subinfo[j].adds*100) / total.subinfo[j].adds;
			stats[i].subinfo[j].subs = (stats[i].subinfo[j].subs*100) / total.subinfo[j].subs;
		}
		stats[i].msginfo.count = (stats[i].msginfo.count*100) / total.msginfo.count;
	}
#endif
}

void usage()
{
	printf(	"ezmlm-stats, version 0.0\n"
			"\n"
			"Usage:\n"
			"\tezmlm-stats [options] /path/to/listdir/\n"
			"\n"
			"Options:\n"
			"\t--startdate date\n"
			"\t--enddate date\n"
			"\t--traffic filename\n"
			"\t--senders filename\n"
			"\t--help, -h\n"
			"\n"
			"Where date is a RFC822 type date, such as 16 May 2000 04:51:35 -0600\n"
			"Where filename can be - (hyphen) for stdout\n"
			"\n");
}

struct option long_options[] =
{
	{ "startdate",	required_argument,	NULL, 1},
	{ "enddate",	required_argument,	NULL, 2},
	{ "traffic",	required_argument,	NULL, 3},
	{ "nosub",		no_argument,		NULL, 6},
	{ "nomsg",		no_argument,		NULL, 7},
	{ "senders",	required_argument,	NULL, 4},
	{ "help",		no_argument,		NULL, 5},
	{ "h",			no_argument,		NULL, 5},
	{0, 0, 0, 0}
};

int main(int argc, char *argv[])
{
	time_t starttime=0, endtime = 0;
	string traffic_fname, senders_fname;


	int option_index = 0;
	int c;
	while ( (c = getopt_long_only(argc, argv, "", long_options, &option_index)) != -1 )
	{
		switch ( c )
		{
			case 1: starttime = datestr2time(optarg); break;
			case 2: endtime = datestr2time(optarg); break;
			case 3: traffic_fname = optarg; break;
			case 4: senders_fname = optarg; break;
			case 5: usage(); return 0;
		}
	}

	if ( optind >= argc ) { printf("Missing listdir arg\n"); return 1; }

	string listdir = argv[optind];

	struct stat statbuf;
	if ( stat(listdir.c_str(), &statbuf) != 0 || !S_ISDIR(statbuf.st_mode) ) { printf("Bad listdir arg\n"); return 1; }

	Cezmlm_stats stats;
	bool gettraffic = traffic_fname.length() > 0;
	bool getsenders = senders_fname.length() > 0;

	if ( !stats.getstats(listdir, starttime, endtime, gettraffic, getsenders) ) { printf("Could not get stats\n"); return 2; }

	if ( gettraffic )
	{
		FILE *fout;
		if ( traffic_fname == "-" ) fout = stdout; else fout = fopen(traffic_fname.c_str(), "wt");
		if ( !fout ) { printf("Could not open traffic output file %s\n", traffic_fname.c_str()); return 3; }

		stats.dumpdailystats(fout);
		if ( fout != stdout ) fclose(fout);
	}
	if ( getsenders )
	{
		FILE *fout;
		if ( senders_fname == "-" ) fout = stdout; else fout = fopen(senders_fname.c_str(), "wt");
		if ( !fout ) { printf("Could not open senders output file %s\n", senders_fname.c_str()); return 4; }

		stats.dumpsenderstats(fout);
		if ( fout != stdout ) fclose(fout);
	}
	
	return 0;
}
