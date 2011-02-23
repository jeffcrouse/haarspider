
#include <iostream>
#include <pcrecpp.h>
#include "string_utils.h"
#include "curl_util.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;
using namespace pcrecpp;

clock_t init;
bool verbose=false;
const char* url=NULL;
const char* haarpath=NULL;
int levels=5;
Size result_size(500, 500);
int min_width=200;
int min_height=96;
int max_total_pages=10000;
vector<string> pages_visited;
vector<string> images_used;
vector<string> bad_suffixes;
Mat result;
cv::CascadeClassifier haarFinder;
float alpha = .1;
float beta = 0;
vector<Mat> faces;


int outputs=0;
string outfile="out.jpg";
string extension;
string basename;


template <class T> string tostr(const T& t);
void parse_args(int argc, char* const argv[]);
void help();
void spider(string url, int level);
bool already_visited(string url);
bool already_added(string url);
void add_to_result(Mat& img);


// -----------------------------------------
int main (int argc, char * const argv[]) {
    
	init=clock();

	// Parse command line options.
	parse_args(argc, argv);
	
	// We can't do anything without a URL and a haar cascade
	if(url==NULL || haarpath==NULL)
	{
		help();
		return 1;
	}
	
	printf("%s --url %s --haarprofile %s --levels %d --outfile %s\n\n", 
		   argv[0], url, haarpath, levels, outfile.c_str());
	
	
	int idx = outfile.rfind('.');
	if(idx == std::string::npos)
	{
		cout << "The output file must have an extension." << endl;
		return 1;
	}
	
	extension = outfile.substr(idx+1);
	basename = outfile.substr(0, idx);
	outfile = basename+tostr(outputs)+"."+extension;
	
	bad_suffixes.push_back(".rss");
	bad_suffixes.push_back(".ico");
	bad_suffixes.push_back(".xml");
	bad_suffixes.push_back(".png");
	bad_suffixes.push_back(".jpg");
	bad_suffixes.push_back(".css");
	bad_suffixes.push_back(".xml");
	
	
	if(!haarFinder.load(haarpath)) {
		cout << "Could not load classifier cascade: " << haarpath << endl;
		return 1;
	}
	
	result.create(result_size, CV_8UC3);
	result = Scalar(255,255,255);
	
	curl_global_init(CURL_GLOBAL_ALL);
	spider(url, 0);
	curl_global_cleanup();
	
	
	cout << "pages visited: " << pages_visited.size() << endl;
	
	
	
	imwrite(outfile, result);
	return 0;
}



// -----------------------------------------
void spider(string url, int level)
{	
	int key = cvWaitKey(1);
	if(key=='o' || key=='O') {
		outputs++;
		outfile = basename+tostr(outputs)+"."+extension;
		imwrite(outfile, result);
	}
	
	// There are several sitautions where we want to stop.
	if(level > levels) return;
	if(pages_visited.size() > max_total_pages) return;
	if(already_visited(url)) return;
	
	// Otherwise, download the URL and add it to the "visited" list
	cout << "opening " << url << "... ";
	string contents = download(url);
	pages_visited.push_back(url);
	cout << "length: " << contents.length() << endl;
	
	// First, pull out any images, defined here as "the src attribute of an img tag"
	string image;
	StringPiece input1(contents);
	string image_pattern="img.src=\"([^\"]+)\"";
	RE image_re(image_pattern, RE_Options().set_caseless(true).set_multiline(true).set_dotall(true).set_match_limit(2000));
	
	while(image_re.FindAndConsume(&input1, &image))
	{
		// If it is a relative link, we add the current url at the beginning.
		if(!starts_with(image, "http://")) {
			image = url + image;
		}
		
		if(already_added(image)) {
			cout << "\t\tImage has already been tried" << endl;
			continue;
		}
		images_used.push_back(image);
		
		char* filename = tmpnam(NULL);
		download(image, filename);
		cout << "\tdownloaded " << image << " to " << filename << endl;
		
		Mat im = imread(filename, 1);
		if(im.empty())
		{
			cout << "\t\tImage did not load" << endl;
		}
		else if(im.size().width < min_width && im.size().height < min_height)
		{
			cout << "\t\tImage is too small" << endl;
		}
		else
		{
			cout << "\t\tAdding to result" << endl;
			add_to_result(im);
		} 
	}
	
	
	// Now find any links, defined here as "any href attribute"
	StringPiece input2(contents);
	string link;
	string link_pattern="href=\"([^\"]+)\"";
	RE link_re(link_pattern, RE_Options().set_caseless(true).set_multiline(true).set_dotall(true).set_match_limit(2000));
	while(link_re.FindAndConsume(&input2, &link))
	{
		// There are several kinds of links we want to ignore.
		if(starts_with(link, "javascript:")) continue;
		if(ends_with(link, bad_suffixes)) continue;

		// If it is a relative link, we add the current url at the beginning.
		if(!starts_with(link, "http://")) {
			link = url + link;
		}
		
		// remove trailing slash
		if(ends_with(link, "/")) {
			link.erase(link.length()-1);
		}
		
		spider(link, level+1);
	}
	
	clock_t elapsed = clock()-init;
	cout << "ELAPSED SECONDS: " << (double)elapsed / ((double)CLOCKS_PER_SEC) << endl;
}



// -----------------------------------------
void add_to_result(Mat& img)
{
	/*
	 void CascadeClassifier::detectMultiScale( const Mat& image, vector<Rect>& objects, double scaleFactor=1.1, int minNeighbors=3, int flags=0, Size minSize=Size());
	 image				Matrix of type CV_8U containing the image in which to detect objects.
	 objects			Vector of rectangles such that each rectangle contains the detected object.
	 scaleFactor		Specifies how much the image size is reduced at each image scale.
	 minNeighbors		Speficifes how many neighbors should each candiate rectangle have to retain it.
	 flags				This parameter is not used for new cascade and have the same meaning for old cascade as in function cvHaarDetectObjects.
	 minSize			The minimum possible object size. Objects smaller than that are ignored.
	 */
	
	vector<cv::Rect> matches;
	haarFinder.detectMultiScale( img, matches);
	
	cout << "\t\t\t" << matches.size() << " faces found" << endl;
	
	Mat resized;
	bool show_preview=false;
	for(int i=0; i<matches.size(); i++)
	{
		resize(img(matches[i]), resized, result_size);
		faces.push_back(resized);
		cout << "TOTAL FACES: " << faces.size() << endl;
		if(faces.size() > 0 && faces.size() % 10 == 0)
			show_preview=true;
	}
	
	if(show_preview)
	{
		float alpha = 1/(float)faces.size();
		for(int i=0; i<faces.size(); i++)
		{
			addWeighted(faces[i], alpha, result, 1.0-alpha, 0, result);
		}
		
		Mat preview = result.clone();
		char label[255];
		sprintf(label, "%d faces", (int)faces.size());
		
		putText(preview, label, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,0));
		imshow("preview", preview);
	}
}
					  
// -----------------------------------------
bool already_visited(string url)
{
	for(int i=0; i<pages_visited.size(); i++) {
		if(url.compare(pages_visited[i])==0) return true;
	}
	return false;
}
	

bool already_added(string url)
{
	for(int i=0; i<images_used.size(); i++) {
		if(url.compare(images_used[i])==0) return true;
	}
	return false;
}


// -----------------------------------------
void help()
{
	cerr << endl;
	cerr << "typical: haarspider (-u|--url ) #### (-p|--profile) #### (-o|--out) ####]" << endl;
	cerr << "  where:" << endl;
	cerr << "  -u (--url) the url on which to start spidering" << endl;
	cerr << "  -o (--out) [default=out.jpg] the file in which to save the result" << endl;
	cerr << "  -m (--max) [default=10000] the maximum number of total pages to spider" << endl;
	cerr << "  -l (--levels) [default=5] the number of levels of links to visit" << endl;
	cerr << "  -p (--profile) the haar profile to use" << endl;
	cerr << "  -h (--help) prints this help message" << endl;
	cerr << "  -v (--verbose) print messages to stderr" << endl;
	cerr << endl;
}


// -----------------------------------------
void parse_args(int argc, char* const argv[])
{
	for(int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--url") == 0)
		{
			if (i+1 == argc) {
				help();
				cerr << "ERROR: Invalid " << argv[i] << " parameter: no url specified" << endl;
				exit(1);
			}
			url = argv[++i];  // parsing action goes here
		}
		else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0)
		{
			if (i+1 == argc) {
				help();
				cerr << "ERROR: Invalid "<<argv[i]<<" parameter: no out file specified"<<endl;
				exit(1);
			}
			outfile = argv[++i];
		}
		else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--max") == 0)
		{
			if (i+1 == argc) {
				help();
				cerr << "ERROR: Invalid "<<argv[i]<<" parameter: no max specified"<<endl;
				exit(1);
			}
			max_total_pages = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--profile") == 0)
		{
			if (i+1 == argc) {
				help();
				cerr << "ERROR: Invalid "<<argv[i]<<" parameter: no haar profile specified"<<endl;
				exit(1);
			}
			haarpath = argv[++i];
		}
		else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--levels") == 0)
		{
			if (i+1 == argc) {
				help();
				cerr << "ERROR: Invalid "<<argv[i]<<" parameter: no levels specified"<<endl;
				exit(1);
			}
			levels = atoi(argv[++i]);
		}
		else if(strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0)
		{
			verbose=true;
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			help();
			exit(0);
		}
	}
}


// -----------------------------------------
template <class T> string tostr(const T& t)
{
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

