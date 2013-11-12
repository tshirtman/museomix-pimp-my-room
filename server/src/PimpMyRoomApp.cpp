#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/Channel.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "cinder/ObjLoader.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Sphere.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Json.h"
#include "cinder/Timeline.h"
#include "cinder/Font.h"
#include "cinder/Path2d.h"
#include "cinder/gl/TileRender.h"

#include "OscListener.h"
//#include "_2RealGStreamerWrapper.h"

#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/date_time/gregorian/parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

#include "WarpBilinear.h"
#include "WarpPerspective.h"
#include "WarpPerspectiveBilinear.h"

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ph::warping;
using namespace boost::filesystem;
//using namespace _2RealGStreamerWrapper;

class PimpMyRoomApp : public AppNative {
  public:
	void setup();
	void update();
	void draw();
	void drawPimp();
	void prepareSettings( Settings *settings );
	void shutdown();
	void resize();
	void mouseMove( MouseEvent event );	
	void mouseDown( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
	bool readJSONInit( const string &path );
	bool saveJSONMask( const fs::path  & path);
	bool readJSONMask( const fs::path  & path);

	void renderTiles();

	wstring GetLocalTime(wstring format);
	gl::TextureRef LoadAsset(string fileName);
	void reg_files( const path & directory, vector<string> &filespath, bool recurse_into_subdirs );
	string GetFileExtension(const string& FileName);
	//void open(fs::path &moviePath);
private:

	params::InterfaceGlRef		mParams;
	ci::CameraPersp				mCamera;

	WarpList					mWarps;
	bool						mUseBeginEnd;
	ci::osc::Listener 				listener;

	//timeline
	ci::TimelineRef mTimeline;

	Vec2f mMouseDownPos;
	// easing
	struct EaseBox {
	  public:
		EaseBox( std::function<float (float)> fn, std::string name )
			: mFn( fn )
		{
		}
	
		std::function<float (float)>	mFn;
	};
	std::vector<EaseBox>		mEaseBoxes;
	//font
	ci::Font			mFont;
	float round(float num, int precision);

	MayaCamUI		mMayaCam;

	bool			mSaveParam;
	bool			mDisplayInfos;

	Vec3f			mAngleInitObj;
	std::string			mtextScenario;

	bool mDisplayPositionObjets;
	Vec3f mEyePoint;
	Vec3f mCenterInterest;
	float mFOV;

	// mask
	std::vector<Path2d>	mPath;
	int		mTrackedPoint;
	int		mMovePoint;
	bool	mDrawMask;
	int		mNumPath;

	struct Image
	{  
		Image() {}
		Image(std::string id , ci::gl::TextureRef image) : mId(id), mImage(image),mZindex(0),mActive(false),mRotate(0.f), mScale(1.f,1.f)  {}
		ci::gl::TextureRef mImage;
		std::string mId;
		Vec2f mPosition;
		Vec2f mScale;
		int mZindex;
		float mRotate;
		bool mActive;

	};
	std::map<std::string, std::map<std::string,Image>> mAsset;
	std::map<std::string, gl::TextureRef> mTextures;
	Vec2f mOffsetPosition;
	float mScaleImage;
	Vec2f mOffsetPositionWall;
	float mScaleImageWall;
	float mOffsetX;
	bool mConsole;
};

/*
void PimpMyRoomApp::open(fs::path &moviePath)
{
	if( ! moviePath.empty() )
	{
		std::shared_ptr<GStreamerWrapper> fileToLoad = std::shared_ptr<GStreamerWrapper>(new GStreamerWrapper());
		std::string uri = "file:/" + moviePath.string();
		if(fileToLoad->open(uri, m_bUseVideoBuffer, m_bUseAudioBuffer))
		{
			m_Players.clear();
			m_Players.push_back(fileToLoad);
			m_Players.back()->setLoopMode(LOOP);
			m_Players.back()->play();
		}
	}
	//mFrameTexture.reset();
}
*/

float PimpMyRoomApp::round(float num, int precision)
{
	return floorf(num * pow (10.f, precision) +.5f)/ pow(10.f, precision);
}

void PimpMyRoomApp::reg_files( const path & directory, vector<string> &filespath, bool recurse_into_subdirs )
	{
	  if( exists( directory ) )
	  {
		directory_iterator end ;
		for( directory_iterator iter(directory) ; iter != end ; ++iter )
		try {
			  if ( is_directory( *iter ) )
			  {
				console() << iter->path().filename() << " (directory)\n" ;
				if( recurse_into_subdirs ) reg_files(*iter, filespath,recurse_into_subdirs) ;
			  }
			  else if ( is_regular_file(*iter))
			  {
				console() << iter->path().filename() << " (file)" ;
				//string filename = iter->path().filename().string();
				string extension = GetFileExtension(iter->path().filename().string());
				if ((extension.compare("jpg") == 0) || (extension.compare("JPG") == 0) || (extension.compare("png") == 0) || (extension.compare("PNG") || (extension.compare("gif") == 0) || (extension.compare("GIF")) == 0))  
				{
					filespath.push_back(iter->path().filename().string());
					console() <<"(image)"<<endl;
				}
				else console() <<"(other)"<<endl;
			  }
			  else
			  {
				console() << iter->path().filename() << " [other]\n";
			  }
		  }
		  catch ( const std::exception & ex )
		  {
			console() << iter->path().filename() << " " << ex.what() << std::endl;
		  }
	  }
	}
string PimpMyRoomApp::GetFileExtension(const string& FileName)
	{
		if(FileName.find_last_of(".") != std::string::npos)
			return FileName.substr(FileName.find_last_of(".")+1);
		return "";
	}
void PimpMyRoomApp::prepareSettings( Settings *settings )
{
	readJSONInit("init.json"); // pour mConsole
	if (mConsole) settings->enableConsoleWindow();
	settings->setResizable( true );
	settings->setWindowSize( 1280, 768 ); // tout les coordonnée sont en résolution 1920.f 1080.f comme le PSD
	settings->setFrameRate(60.f);
	settings->setFullScreen();
}

bool PimpMyRoomApp::saveJSONMask( const fs::path  & path)
{
	 // Add an array of objects (append value to itself three times)

	try {
	JsonTree shapes = JsonTree::makeArray("shapesmask");

	for ( int i = 0 ; i < mPath.size() ; ++i)
	{
		JsonTree shape = JsonTree::makeArray("shape" + boost::lexical_cast<string> (i));
		for( size_t p = 0; p < mPath.at(mNumPath).getNumPoints(); ++p )
		{
			JsonTree point = JsonTree::makeArray("point" + boost::lexical_cast<string> (p));
			point.pushBack(JsonTree("x", mPath.at(i).getPoint(p).x));
			point.pushBack(JsonTree("y", mPath.at(i).getPoint(p).y));
			shape.pushBack(point);
		}
		shapes.pushBack(shape);
	}

	//fs::path localFile = getAssetPath("") / path;
	shapes.write( path );
	} catch(...) {
		console()<<" error serialise shapes " <<endl; 
		return false;
	}

	return true;
}

bool PimpMyRoomApp::readJSONMask( const fs::path  & path)
{
	if ( !fs::exists( path ) ) 
	{
		console()<<("fichier json non trouvé: " + path.string()); 
		return false;
	}
	JsonTree shapes( loadFile( path ) );

	try {
	//JsonTree shapes = json.getChild("shapesmask");

	for( JsonTree::ConstIter shape = shapes.begin(); shape != shapes.end(); ++shape ) 
	{
		mPath.push_back(Path2d());
		int numPoint = 0;
		for( JsonTree::ConstIter point = shape->begin(); point != shape->end(); ++point ) 
		{
			if (numPoint == 0) mPath.back().moveTo(Vec2f(point->getChild("x").getValue<float>(), point->getChild("y").getValue<float>()));
			else mPath.back().lineTo( Vec2f(point->getChild("x").getValue<float>(), point->getChild("y").getValue<float>()));
			numPoint ++;
		}
	}
	} catch(...) 
	{
		console()<<"error read json mask" << endl;
	}
	return true;
}

bool PimpMyRoomApp::readJSONInit( const std::string &path )
{
	try 
	{
		string pathToFile = ( getAssetPath("") / path ).string();
		if ( !fs::exists( pathToFile ) ) 
		{
			console()<<("fichier json non trouvé: " + path); 
			return false;
		}
		JsonTree json( loadFile( pathToFile ) );
		JsonTree visu = json.getChild( "config.visu" ); 

		//visu
		mOffsetX = visu.getChild( "mOffsetX" ).getValue<float>();
		mSaveParam = visu.getChild( "mSaveParam" ).getValue<bool>();
		mConsole = visu.getChild( "mConsole" ).getValue<bool>();
		mDisplayInfos = visu.getChild( "mDisplayInfos" ).getValue<bool>(); 
		mOffsetPosition = Vec2f(visu.getChild( "mOffsetPositionImage" ).getChild("x").getValue<float>(), visu.getChild( "mOffsetPositionImage" ).getChild("y").getValue<float>()) ;
		mScaleImage = visu.getChild( "mScaleImage" ).getValue<float>();
		mOffsetPositionWall = Vec2f(visu.getChild( "mOffsetPositionWall" ).getChild("x").getValue<float>(), visu.getChild( "mOffsetPositionWall" ).getChild("y").getValue<float>());
		mScaleImageWall = visu.getChild( "mScaleImageWall" ).getValue<float>();

	} 
	catch ( ... ) 
	{
		console()<<"Erreur sur fichier JSON: " + path<<endl;
		return false;
	}
	return true;
}

gl::TextureRef PimpMyRoomApp::LoadAsset(string fileName)
{
	console()<<"loadAsset fileName =" + fileName;
	gl::TextureRef glTexture = NULL;
	if(fileName != "")//&&( fs::exists(loadAsset(fileName)->getFilePathHint())))
	{
		try {
			glTexture = gl::Texture::create( loadImage(  loadAsset(fileName) ) );
			mTextures[fileName] = glTexture ;
		}
		catch( ... ) {
			console()<<"AssetManager.cpp : unable to load the texture file " + fileName + " !";
		}
	}
	return glTexture;
}

void PimpMyRoomApp::shutdown()
{
	//renderTiles();
	// save warp settings
	fs::path settings = getAssetPath("") / "warps.xml";
	Warp::writeSettings( mWarps, writeFile( settings ) );
	fs::path shapesmask = getAssetPath("") / "shapesmask.json";
	saveJSONMask(shapesmask);
}

void PimpMyRoomApp::resize()
{
	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize( mWarps );

	//mGizmo->resize();
}

void PimpMyRoomApp::mouseMove( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( ! Warp::handleMouseMove( mWarps, event ) )
	{
		// let your application perform its mouseMove handling here
	}
	//mGizmo->mouseMove(event);
}

void PimpMyRoomApp::mouseDown( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( ! Warp::handleMouseDown( mWarps, event ) )
	{
		// let your application perform its mouseDown handling here
	}
	mMouseDownPos =  event.getPos();

	if(mDrawMask && mPath.size() > 0)
	{
		//Mask
		if( event.isLeftDown() ) 
		{ // line

			for( size_t p = 0; p < mPath.at(mNumPath).getNumPoints(); ++p )
			{
				float dist = event.getPos().distance(mPath.at(mNumPath).getPoint(p));
				if (dist < 20.f)
				{
					mMovePoint = p;
					break;
				}

			}


			if (mMovePoint == -1)
			{
				{
					if( mPath.at(mNumPath).empty() ) {
						mPath.at(mNumPath).moveTo( event.getPos() );
						mTrackedPoint = 0;
					}
					else
						mPath.at(mNumPath).lineTo( event.getPos() );
				}
			}
		}

	}

	//mGizmo->mouseDown(event);
}

void PimpMyRoomApp::mouseDrag( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( ! Warp::handleMouseDrag( mWarps, event ) )
	{
		// let your application perform its mouseDrag handling here
	}

	// mask
	if(mDrawMask && mPath.size() > 0)
	{
		if(mMovePoint >= 0) {
			mPath.at(mNumPath).getPoint(mMovePoint).set(event.getPos() );
		}
	}
}

void PimpMyRoomApp::mouseUp( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( ! Warp::handleMouseUp( mWarps, event ) )
	{
		// let your application perform its mouseUp handling here
	}

	//mask
	mTrackedPoint = -1;
	mMovePoint = -1;
}

void PimpMyRoomApp::keyDown( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( ! Warp::handleKeyDown( mWarps, event ) )
	{
		// warp editor did not handle the key, so handle it here
		switch( event.getCode() )
		{
		case KeyEvent::KEY_ESCAPE:
			// quit the application
			quit();
			break;
		case KeyEvent::KEY_f:
			// toggle full screen
			setFullScreen( ! isFullScreen() );
			SetWindowPos( getWindow()->getRenderer()->getHwnd() , HWND_TOPMOST, mOffsetX,0,getWindowWidth(),getWindowHeight(), SWP_SHOWWINDOW);
			break;
		case KeyEvent::KEY_w:
			// toggle warp edit mode
			Warp::enableEditMode( ! Warp::isEditModeEnabled() );
			break;
		case KeyEvent::KEY_SPACE:
			// toggle drawing mode
			mUseBeginEnd = !mUseBeginEnd;
			break;
		case KeyEvent::KEY_1: 
			//mGizmo->setMode( Gizmo::TRANSLATE );
			break;
		case KeyEvent::KEY_2: 
			//mGizmo->setMode( Gizmo::ROTATE );
			break;
		case KeyEvent::KEY_3: 
			//mGizmo->setMode( Gizmo::SCALE );
			break;
		case KeyEvent::KEY_o:  
		{
			//CameraPersp centered = mMayaCam.getCamera();
			//centered.setCenterOfInterestPoint( mGizmo->getTranslate() );
			//mMayaCam.setCurrentCam( centered );
			break;
		}
		case KeyEvent::KEY_i: 
			mDisplayInfos = !mDisplayInfos; 
			break;
		case  KeyEvent::KEY_x :
		{
			if ( mPath.size() > 0)
			{
				mPath.at(mNumPath).clear();
				mPath.erase(mPath.begin() + mNumPath);
				if (mPath.size() != 0) mNumPath %= mPath.size();
				else mNumPath = 0;
			}
			break;
		}
		case KeyEvent::KEY_v :
			mDrawMask = !mDrawMask;
		break;
		case KeyEvent::KEY_n :
			mPath.push_back(Path2d());
			mNumPath ++;
			if (mPath.size() != 0) mNumPath %= mPath.size();
			else mNumPath = 0;
			break;
		case KeyEvent::KEY_b :
			mNumPath ++;
			if (mPath.size() != 0) mNumPath %= mPath.size();
			else mNumPath = 0;
			break;
		}
	}
}


void PimpMyRoomApp::keyUp( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( ! Warp::handleKeyUp( mWarps, event ) )
	{
		// let your application perform its keyUp handling here
	}
}
void PimpMyRoomApp::setup()
{


	SetWindowPos(getWindow()->getRenderer()->getHwnd() , HWND_TOPMOST, mOffsetX,0,getWindowWidth(),getWindowHeight(), SWP_SHOWWINDOW);
	//read JSON
	fs::path shapesmask = getAssetPath("") / "shapesmask.json";
	readJSONMask(shapesmask);
	fs::path pathFond = getAssetPath("") / "finale" / "fonds";
	vector<string> filesFond;
	reg_files (pathFond, filesFond, false);
	for (int i = 0 ; i < filesFond.size() ; ++i)
	{
		mAsset["wallpaper"][filesFond.at(i)] = Image(filesFond.at(i), LoadAsset("finale\\fonds\\" + filesFond.at(i)));
		console()<<"filesFond.at(i).filename().string()"<<filesFond.at(i)<<endl;
	}

	fs::path pathImage = getAssetPath("") / "finale" / "objets";
	vector<string> filesImage;
	reg_files (pathImage, filesImage, false);
	for (int i = 0 ; i < filesImage.size() ; ++i)
	{
		mAsset["image"][filesImage.at(i)] = Image(filesImage.at(i), LoadAsset("finale\\objets\\" + filesImage.at(i)));
		console()<<"filesImage.at(i).filename().string()"<<filesImage.at(i)<<endl;
	}


	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 1.0f, 5000.0f );
	mCamera.lookAt( ci::Vec3f::zero(), ci::Vec3f::zAxis(), ci::Vec3f::yAxis() );

	mEyePoint = Vec3f(0.66f,0.03f,100.f);
	mFOV = 45.f;
	mCenterInterest = Vec3f(7.74f,4.22f,-38.55f);
	CameraPersp initialCam;
	initialCam.setPerspective( mFOV, getWindowAspectRatio(), 1.f, 10000 );
	initialCam.setEyePoint(mEyePoint);
	initialCam.setCenterOfInterestPoint(mCenterInterest);
	mMayaCam.setCurrentCam( initialCam );

	// params
	mParams = params::InterfaceGl::create( "Réglages", ci::Vec2i( 280, 200 ) );
	mParams->setOptions( "", "position='" + boost::lexical_cast<string>(50) + " " + boost::lexical_cast<string>(400) + "'" );

	//mParams->addParam("position eye", &mEyePoint);
	//mParams->addParam("position center interest", &mCenterInterest);
	//mParams->addParam("FOV", &mFOV); 
	//mParams->addParam( "SCENARIO", &mtextScenario);



	mParams->addParam( "mOffsetPosition image  X", &mOffsetPosition.x, "step=0.1f");
	mParams->addParam( "mOffsetPosition image  Y", &mOffsetPosition.y, "step=0.1f");
	mParams->addParam( "Scale object", &mScaleImage,"step=0.01f");

	mParams->addParam( "mOffsetPosition wallpaper X", &mOffsetPositionWall.x, "step=0.1f");
	mParams->addParam( "mOffsetPosition wallpaper Y", &mOffsetPositionWall.y, "step=0.1f");
	mParams->addParam( "Scale wallpaper", &mScaleImageWall,"step=0.01f");

	
	mDisplayPositionObjets = false;

	// initialize warps
	fs::path settings = getAssetPath("") / "warps.xml";
	if( fs::exists( settings ) )
	{
		// load warp settings from file if one exists
		mWarps = Warp::readSettings( loadFile( settings ) );
	}
	else
	{
		// otherwise create a warp from scratch
		mWarps.push_back( WarpPerspectiveBilinearRef( new WarpPerspectiveBilinear() ) );
		//mWarps.push_back( WarpPerspectiveRef( new WarpPerspective() ) );
		//mWarps.push_back( WarpBilinearRef( new WarpBilinear() ) );
	}
			// adjust the content size of the warps
	Warp::setSize( mWarps, Vec2f(1920.f, 1080.f)/*mXBoxTextureRef->getSize()*/ /*getWindowSize()*/ );
	mUseBeginEnd = true;

	//OSC
	listener.setup( 3000 );

	// init easing
	mEaseBoxes.push_back( EaseBox( EaseInQuad(), "EaseInQuad" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutQuad(), "EaseOutQuad" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutQuad(), "EaseInOutQuad" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInQuad(), "EaseOutInQuad" ) );

	mEaseBoxes.push_back( EaseBox( EaseInCubic(), "EaseInCubic" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutCubic(), "EaseOutCubic" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutCubic(), "EaseInOutCubic" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInCubic(), "EaseOutInCubic" ) );

	mEaseBoxes.push_back( EaseBox( EaseInQuart(), "EaseInQuart" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutQuart(), "EaseOutQuart" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutQuart(), "EaseInOutQuart" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInQuart(), "EaseOutInQuart" ) );

	mEaseBoxes.push_back( EaseBox( EaseInQuint(), "EaseInQuint" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutQuint(), "EaseOutQuint" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutQuint(), "EaseInOutQuint" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInQuint(), "EaseOutInQuint" ) );

	mEaseBoxes.push_back( EaseBox( EaseInSine(), "EaseInSine" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutSine(), "EaseOutSine" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutSine(), "EaseInOutSine" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInSine(), "EaseOutInSine" ) );

	mEaseBoxes.push_back( EaseBox( EaseInExpo(), "EaseInExpo" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutExpo(), "EaseOutExpo" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutExpo(), "EaseInOutExpo" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInExpo(), "EaseOutInExpo" ) );

	mEaseBoxes.push_back( EaseBox( EaseInCirc(), "EaseInCirc" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutCirc(), "EaseOutCirc" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutCirc(), "EaseInOutCirc" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInCirc(), "EaseOutInCirc" ) );

	mEaseBoxes.push_back( EaseBox( EaseInAtan(), "EaseInAtan" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutAtan(), "EaseOutAtan" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutAtan(), "EaseInOutAtan" ) );
	mEaseBoxes.push_back( EaseBox( EaseNone(), "EaseNone" ) );

	mEaseBoxes.push_back( EaseBox( EaseInBack(), "EaseInBack" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutBack(), "EaseOutBack" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutBack(), "EaseInOutBack" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInBack(), "EaseOutInBack" ) );

	mEaseBoxes.push_back( EaseBox( EaseInBounce(), "EaseInBounce" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutBounce(), "EaseOutBounce" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutBounce(), "EaseInOutBounce" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInBounce(), "EaseOutInBounce" ) );

	mEaseBoxes.push_back( EaseBox( EaseInElastic( 2, 1 ), "EaseInElastic(2, 1)" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutElastic( 1, 4 ), "EaseOutElastic(1, 4)" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutElastic( 2, 1 ), "EaseInOutElastic( 2, 1 )" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInElastic( 1, 4 ), "EaseOutInElastic( 4, 1 )" ) );

	// mask
	mMovePoint = -1;

	// affiche
	mFont = Font( "Arial", 40.f );

	//mask
	mDrawMask = false;
	mParams->addParam( "Draw mask", &mDrawMask );
	mNumPath = 0;

	// video
	//m_bUseVideoBuffer = true;
	//m_bUseAudioBuffer = false;
	//open(fs::path(mVideoPath.at(0)));
}   

wstring PimpMyRoomApp::GetLocalTime(wstring format)//wtime_facet * facet
{
	wtime_facet * facet = new wtime_facet;// pas de () pour éviter fuite de mémoire (ceci est particulier voir http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/) ne peut pas se supprimer "colin %A-%d-%B-%Y %H:%M:%S"
	facet->format(format.c_str());
	//facet->long_month_names(mLong_months);
    //facet->long_weekday_names(mLong_days);
	
	wstringstream ss;
	ss.str(L"");
	ss.imbue(locale(ss.getloc(), facet));
	ss << second_clock::local_time() << endl;
	wstring ret = ss.str();
	//console() <<toUtf8(format)<<" = "<< toUtf8(ret)<< endl;

	return ret;
}

void PimpMyRoomApp::renderTiles()
{
	//the size here doesn't matter, but it will get distorted if it's not the same ratio as your window
	
	gl::TileRender tr( getWindowWidth(), getWindowHeight());
	//use the default cinder view to render from
	//tr.setMatricesWindowPersp(getWindowSize()*0.5f);
	tr.setMatricesWindow( getWindowSize() /*mMayaCam.getCamera()*/) ;//getWindowWidth(), getWindowHeight());
	//tr.setMatricesWindow(getWindowWidth(), getWindowHeight());
	while( tr.nextTile() ) {
		drawPimp();;
	}
	
	string time = toUtf8(GetLocalTime(L"%H-%M-%S"));
	string time2;
	try {
	time2 = time.substr(0, time.size()-1);
	} catch (...) { 
		console()<<"error get date"<<endl;
	}
	//Surface snapshot = copyWindowSurface();
	string link =  getHomeDirectory().string() /* + getPathSeparator()*/ + "snapshotPimp" + getPathSeparator() + "snapshot_" + time2 +".png"; //::getAssetPath("snapshotPimp").string()
	writeImage(link , /*snapshot*/ tr.getSurface() ); //%A-%D-%B-%Y_

}

void PimpMyRoomApp::update()
{

	CameraPersp initialCam;
	initialCam.setPerspective( mFOV, getWindowAspectRatio(), 1.f, 10000 );
	initialCam.setEyePoint(mEyePoint);
	initialCam.setCenterOfInterestPoint(mCenterInterest);
	mMayaCam.setCurrentCam( initialCam );

	
	while( listener.hasWaitingMessages() ) {
		osc::Message message;
		listener.getNextMessage( &message );
		
		/*
		console() << "New message received" << std::endl;
		console() << "Address: " << message.getAddress() << std::endl;
		console() << "Num Arg: " << message.getNumArgs() << std::endl;
		for (int i = 0; i < message.getNumArgs(); i++) {
			console() << "-- Argument " << i << std::endl;
			console() << "---- type: " << message.getArgTypeName(i) << std::endl;
			if( message.getArgType(i) == osc::TYPE_INT32 ) {
				try {
					console() << "------ value: "<< message.getArgAsInt32(i) << std::endl;
				}
				catch (...) {
					console() << "Exception reading argument as int32" << std::endl;
				}
			}
			else if( message.getArgType(i) == osc::TYPE_FLOAT ) {
				try {
					console() << "------ value: " << message.getArgAsFloat(i) << std::endl;
				}
				catch (...) {
					console() << "Exception reading argument as float" << std::endl;
				}
			}
			else if( message.getArgType(i) == osc::TYPE_STRING) {
				try {
					console() << "------ value: " << message.getArgAsString(i).c_str() << std::endl;
				}
				catch (...) {
					console() << "Exception reading argument as string" << std::endl;
				}
			}
		}*/
        
		if (message.getAddress() == "/create") 
		{
			if( message.getNumArgs() > 5 && message.getArgType( 0 ) == osc::TYPE_STRING && message.getArgType( 1 ) == osc::TYPE_FLOAT && message.getArgType( 2 ) == osc::TYPE_FLOAT && message.getArgType( 3 ) == osc::TYPE_INT32 && message.getArgType( 4 ) == osc::TYPE_FLOAT && message.getArgType( 5 ) == osc::TYPE_FLOAT)
			{
				string id = message.getArgAsString(0);
				console()<<"create"<<endl;
				//string id = "mucha";
				float x = message.getArgAsFloat(1);
				float y = message.getArgAsFloat(2);
				int z = message.getArgAsInt32(3);
				float scale = message.getArgAsFloat(4);
				float rotate = message.getArgAsFloat(5);
				mAsset["image"][id].mId = id;
				mAsset["image"][id].mPosition = lmap(Vec2f(x, y), Vec2f(0.f,0.f), Vec2f(1.f,1.f), -Vec2f(getWindowSize().x*0.5f, getWindowSize().y*0.5f), Vec2f(getWindowSize().x*0.5f, getWindowSize().y*0.5f));
				mAsset["image"][id].mZindex = z;
				mAsset["image"][id].mRotate = rotate; 
				mAsset["image"][id].mScale = Vec2f(scale, scale);
				mAsset["image"][id].mActive = true;
			}
		}
		else if (message.getAddress() == "/update") 
		{
			if( message.getNumArgs() > 5 && message.getArgType( 0 ) == osc::TYPE_STRING && message.getArgType( 1 ) == osc::TYPE_FLOAT && message.getArgType( 2 ) == osc::TYPE_FLOAT && message.getArgType( 3 ) == osc::TYPE_INT32 && message.getArgType( 4 ) == osc::TYPE_FLOAT && message.getArgType( 5 ) == osc::TYPE_FLOAT)
			{
				string id = message.getArgAsString(0);
				//string id = "mucha";
				float x = message.getArgAsFloat(1);
				float y = message.getArgAsFloat(2);
				int z = message.getArgAsInt32(3);
				float scale = message.getArgAsFloat(4);
				float rotate = message.getArgAsFloat(5);
				mAsset["image"][id].mId = id;
				mAsset["image"][id].mPosition = lmap(Vec2f(x, y), Vec2f(0.f,0.f), Vec2f(1.f,1.f), -Vec2f(getWindowSize().x*0.5f, getWindowSize().y*0.5f), Vec2f(getWindowSize().x*0.5f, getWindowSize().y*0.5f));
				mAsset["image"][id].mZindex = z;
				mAsset["image"][id].mRotate = rotate;
				mAsset["image"][id].mScale = Vec2f(scale, scale);
				mAsset["image"][id].mActive = true;
			}
		}
		else if (message.getAddress() == "/delete") 
		{
			if( message.getNumArgs() > 0 && message.getArgType( 0 ) == osc::TYPE_STRING)
			{
				console()<<"delete"<<endl;
				//string id = message.getArgAsString(0);
				string id = "mucha";
				mAsset["image"][id].mActive = false;
			}
		}
		else if (message.getAddress() == "/wallpaper") 
		{
			if( message.getNumArgs() > 0 && message.getArgType( 0 ) == osc::TYPE_STRING)
			{
				console()<<"wallpaper"<<endl;
				string id = message.getArgAsString(0);
				//string id = "02";
				for( auto iter = mAsset["wallpaper"].begin() ; iter != mAsset["wallpaper"].end() ; ++iter)
				{
					iter->second.mActive = false;
				}
				mAsset["wallpaper"][id].mActive = true;
			}
		}
		else if (message.getAddress() == "/clearall") 
		{
			for( auto iter = mAsset["wallpaper"].begin() ; iter != mAsset["wallpaper"].end() ; ++iter)
			{
				iter->second.mActive = false;
			}
			for( auto iter = mAsset["image"].begin() ; iter != mAsset["image"].end() ; ++iter)
			{
				iter->second.mActive = false;
			}

		}

	}
	
}

void PimpMyRoomApp::drawPimp()
{
	gl::clear( ColorAf::black() );
	gl::enableAlphaBlending();
	gl::enableDepthWrite();
	gl::enableDepthRead();
	glDisable( GL_CULL_FACE );
	glAlphaFunc(GL_GREATER, 0.05);
	glEnable(GL_ALPHA_TEST);

	gl::pushMatrices();
	gl::setMatricesWindow( getWindowSize() );
	gl::color(Colorf::white());
	for ( auto iter = mAsset["wallpaper"].begin() ; iter != mAsset["wallpaper"].end() ; ++iter)
	{
		if (iter->second.mActive)
		{
			gl::pushMatrices();
			gl::translate(getWindowSize() * 0.5f);
			gl::translate(mOffsetPositionWall.x,mOffsetPositionWall.y,0.f);
			gl::scale(mScaleImageWall , mScaleImageWall , 1.f);
			if (iter->second.mImage ) gl::draw( iter->second.mImage, - Vec2f(iter->second.mImage->getSize() * 0.5f));
			gl::popMatrices();
		}
	}
				
	for ( auto iter = mAsset["image"].begin() ; iter != mAsset["image"].end() ; ++iter)
	{
		if (iter->second.mActive)
		{
			gl::pushMatrices();
			gl::translate(getWindowSize() * 0.5f);
			gl::translate( mOffsetPosition.x + iter->second.mPosition.x , mOffsetPosition.y + iter->second.mPosition.y, 0.1f + (float) iter->second.mZindex *0.01f);
			gl::scale(mScaleImage* iter->second.mScale.x , mScaleImage* iter->second.mScale.y , 1.f) ;
			gl::rotate(iter->second.mRotate);
			if (iter->second.mActive && iter->second.mImage ) gl::draw( iter->second.mImage, - Vec2f(iter->second.mImage->getSize() * 0.5f));
			gl::popMatrices();
		}
	}
	gl::popMatrices();
}
void PimpMyRoomApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::clear( ColorAf::black() );
	gl::color(ColorAf::white());

	gl::setMatricesWindow( getWindowSize() );
	gl::enableAlphaBlending();
	// iterate over the warps and draw their content
	for(WarpConstIter itr=mWarps.begin();itr!=mWarps.end();++itr)
	{
		// create a readable reference to our warp, to prevent code like this: (*itr)->begin();
		WarpRef warp( *itr );

		// there are two ways you can use the warps:
		if( mUseBeginEnd )
		{
			// a) issue your draw commands between begin() and end() statements
			warp->begin();
				//gl::setMatrices( mMayaCam.getCamera() );
				gl::setMatricesWindow( getWindowSize() );
				drawPimp();
			warp->end();
		}
	}
	gl::setMatricesWindow( getWindowSize() );
	gl::enableAlphaBlending();
	if(mDrawMask && mPath.size() > 0)
	{
		// draw the control points
		gl::color( ColorA( 1, 1, 0 , 1) );
		for( size_t p = 0; p < mPath.at(mNumPath).getNumPoints(); ++p )
			gl::drawSolidCircle( mPath.at(mNumPath).getPoint( p ), 2.5f );

		// draw the precise bounding box
		/*gl::color( ColorA( 0, 1, 1, 0.2f ) );
		gl::drawSolidRect( mPath.calcPreciseBoundingBox() );
		*/

		// draw the curve itself
		gl::color( ColorA( 1.0f, 0.5f, 0.25f , 0.8f) );
		gl::draw( mPath.at(mNumPath) );

		//draw interaction point
		gl::color(ColorA(1.f,1.f,0.f,0.5f));
		for( size_t p = 0; p < mPath.at(mNumPath).getNumPoints(); ++p )
		{
			if (p != mMovePoint )
				gl::drawSolidCircle(mPath.at(mNumPath).getPoint(p), 10.f);
		}

		//draw move point
		if(mMovePoint != -1)
		{
			gl::color(ColorA(1.f,0.1f,0.f,0.5f));
			gl::drawSolidCircle(mPath.at(mNumPath).getPoint(mMovePoint), 10.f);
		}
	}

	if( mPath.size() > 0 )
	{
		gl::color( Color::black());
		for(int i = 0 ; i < mPath.size() ; ++i)
		{
			if (mPath.at(i).getNumPoints() > 3)
			{
				gl::drawSolid(mPath.at(i));
			}
		}
	}

	if (mDisplayInfos)
	{
		if(mParams) mParams->draw();
	}

}

CINDER_APP_NATIVE( PimpMyRoomApp, RendererGl )
