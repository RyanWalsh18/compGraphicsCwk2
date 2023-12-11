#include <glad.h>
#include <GLFW/glfw3.h>

#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "defaults.hpp"
#include "loadobj.hpp"
#include "texture.hpp"

#include <iostream>

#include <rapidobj/rapidobj.hpp>

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";
	
	constexpr float kPi_ = 3.1415926f;

	float kMovementPerSecond_ = 5.f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

	struct State_
	{
		ShaderProgram* prog;
		ShaderProgram* progTextures;

		struct CamCtrl_
		{
			bool cameraActive;
			bool actionMoveForward, actionMoveBackward;
			bool actionMoveLeft, actionMoveRight;
			bool actionMoveUp, actionMoveDown;
			bool actionSprint, actionCrouch;

			float phi, theta;
			float radius;
			float x, y, z;

			float lastX, lastY;
		} camControl;
	};

	void glfw_callback_error_(int, char const*);

	void glfw_callback_key_(GLFWwindow*, int, int, int, int);
	void glfw_callback_motion_(GLFWwindow*, double, double);

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};
}

int main() try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '%s' (%d)", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '%s' (%d)", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };

	// Set up event handling
	// TODO: Additional event handling setup
	
	State_ state{};

	glfwSetWindowUserPointer( window, &state );

	glfwSetKeyCallback(window, &glfw_callback_key_);
	glfwSetCursorPosCallback(window, &glfw_callback_motion_);

	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoaDGLLoader() failed - cannot load GL API!" );

	std::printf( "RENDERER %s\n", glGetString( GL_RENDERER ) );
	std::printf( "VENDOR %s\n", glGetString( GL_VENDOR ) );
	std::printf( "VERSION %s\n", glGetString( GL_VERSION ) );
	std::printf( "SHADING_LANGUAGE_VERSION %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	//load the shader program

	ShaderProgram prog( {
		{ GL_VERTEX_SHADER, "assets/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/default.frag" }
	} );

	ShaderProgram progTextures( {
		{ GL_VERTEX_SHADER, "assets/textures.vert" },
		{ GL_FRAGMENT_SHADER, "assets/textures.frag" }
	} );

	state.prog = &prog;
	state.progTextures = &progTextures;
	state.camControl.radius = 10.f;

	OGL_CHECKPOINT_ALWAYS();
	//create VBOs and VAOs
	SimpleMeshData finland = load_wavefront_obj("assets/parlahti.obj");
	GLuint finlandVAO = create_vao(finland);
	std::size_t finlandVertexCount = finland.positions.size();

	std::cout << "test \n";
	GLuint finlandTexture = load_texture_2d("assets/L4343A-4k.jpeg");
	std::cout << "test good \n";
	SimpleMeshData landingPad = load_wavefront_obj("assets/landingpad.obj");

	GLuint landingPadVAO = create_vao(landingPad);
	std::size_t landingPadVertexCount = landingPad.positions.size();

	// end of VBOs and VAOs

	// Animation state
	auto last = Clock::now();

	float angle = 0.f;

	OGL_CHECKPOINT_ALWAYS();
	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, nwidth, nheight );
		}

		// Update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now-last).count();
		last = now;

		static constexpr float kPi_ = 3.1415926f; // this is me being lazy.
		angle += dt * kPi_ * 0.3f;
		if( angle >= 2.f*kPi_ )
			angle -= 2.f*kPi_;
		// Update camera state for movement
		if (state.camControl.actionMoveForward)
		{
			// Calculate movement direction in camera space
			Vec3f forwardVec = normalize(Vec3f{
				std::sin(state.camControl.phi),
				std::sin(state.camControl.theta),
				-(std::cos(state.camControl.phi))
			});

			state.camControl.x += forwardVec.x * kMovementPerSecond_ * dt;
			state.camControl.y -= forwardVec.y * kMovementPerSecond_ * dt;
			state.camControl.z += forwardVec.z * kMovementPerSecond_ * dt;
		}
		if (state.camControl.actionMoveBackward)
		{
			Vec3f backwardVec = -normalize(Vec3f{
				std::sin(state.camControl.phi),
				std::sin(state.camControl.theta),
				-(std::cos(state.camControl.phi))
			});

			state.camControl.x += backwardVec.x * kMovementPerSecond_ * dt;
			state.camControl.y -= backwardVec.y * kMovementPerSecond_ * dt;
			state.camControl.z += backwardVec.z * kMovementPerSecond_ * dt;
		}
		if (state.camControl.actionMoveLeft)
		{
			// Calculate the left direction vector in camera space
			Vec3f leftVec = normalize(cross(Vec3f{0.0f, 1.0f, 0.0f}, normalize(Vec3f{
				std::sin(state.camControl.phi),
				std::sin(state.camControl.theta),
				-(std::cos(state.camControl.phi))
			})));

			state.camControl.x += leftVec.x * kMovementPerSecond_ * dt;
			state.camControl.y += leftVec.y * kMovementPerSecond_ * dt;
			state.camControl.z += leftVec.z * kMovementPerSecond_ * dt;
		}
		if (state.camControl.actionMoveRight)
		{
			Vec3f rightVec = -normalize(cross(Vec3f{0.0f, 1.0f, 0.0f}, normalize(Vec3f{
				std::sin(state.camControl.phi),
				std::sin(state.camControl.theta),
				-(std::cos(state.camControl.phi))
			})));

			state.camControl.x += rightVec.x * kMovementPerSecond_ * dt;
			state.camControl.y += rightVec.y * kMovementPerSecond_ * dt;
			state.camControl.z += rightVec.z * kMovementPerSecond_ * dt;
		}
		if (state.camControl.actionMoveUp)
		{
			state.camControl.y += kMovementPerSecond_ * dt;
		}
		if (state.camControl.actionMoveDown)
		{
			state.camControl.y -= kMovementPerSecond_ * dt;
		}
		if (state.camControl.actionSprint)
		{
			kMovementPerSecond_ = 10.f;
		}
		else if (state.camControl.actionCrouch)
		{
			kMovementPerSecond_ = 2.5f;
		}
		else
		{
			kMovementPerSecond_ = 5.f;
		}


		if (state.camControl.radius <= 0.1f)
			state.camControl.radius = 0.1f;

		//matrix functions
		Mat44f Rx = make_rotation_x(state.camControl.theta);
		Mat44f Ry = make_rotation_y(state.camControl.phi);
		Mat44f T = make_translation({0.0f, 0.0f, -10.f});
		Mat44f T_pad = make_translation({21.0f, -0.95f, -10.f});
		Mat44f T_pad2 = make_translation({50.0f, -0.9f, 3.f});
		Mat44f model2world = make_translation({-state.camControl.x,-state.camControl.y,-state.camControl.z});
		Mat44f world2camera = Rx * Ry ;
		Mat44f projection = make_perspective_projection(
			60.f * 3.1415926f / 180.f,
			fbwidth / float(fbheight),
			0.1f, 100.0f);
		Mat44f projCameraWorld = projection  * world2camera * model2world;
		Mat44f projCameraWorld_pad = projection  * world2camera * model2world * T_pad;
		Mat44f projCameraWorld_pad2 = projection  * world2camera * model2world * T_pad2;
		Mat33f normalMatrix = mat44_to_mat33( transpose(invert(T)) );

		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//TODO: draw frame
		glUseProgram( progTextures.programId() );
		Vec3f lightDir = normalize( Vec3f{-1.f, 1.f, 0.5f});
		glUniform3fv(2,1, &lightDir.x);
		glUniform3f(3, 0.9f, 0.9f, 0.6f);
		glUniform3f(4, 0.05f, 0.05f, 0.05f);
		glUniformMatrix3fv(
			1,
			1, GL_TRUE, normalMatrix.v
		);

		OGL_CHECKPOINT_DEBUG();

		glUniformMatrix4fv(
			0,
			1, GL_TRUE, projCameraWorld.v
		);
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, finlandTexture );
		glBindVertexArray( finlandVAO );

		glDrawArrays(GL_TRIANGLES, 0, finlandVertexCount);

		glBindVertexArray( 0 );
		glUseProgram( 0 );
		glUseProgram( prog.programId() );

		glUniform3fv(2,1, &lightDir.x);
		glUniform3f(3, 0.9f, 0.9f, 0.6f);
		glUniform3f(4, 0.05f, 0.05f, 0.05f);
		glUniformMatrix3fv(
			1,
			1, GL_TRUE, normalMatrix.v
		);

		OGL_CHECKPOINT_DEBUG();

		glUniformMatrix4fv(
			0,
			1, GL_TRUE, projCameraWorld_pad.v
		);

		glBindVertexArray( landingPadVAO );
		glDrawArrays(GL_TRIANGLES, 0, landingPadVertexCount);

		glBindVertexArray( 0 );
		glUseProgram( 0 );
		glUseProgram( prog.programId() );
		glUniformMatrix4fv(
			0,
			1, GL_TRUE, projCameraWorld_pad2.v
		);

		glBindVertexArray( landingPadVAO );
		glDrawArrays(GL_TRIANGLES, 0, landingPadVertexCount);

		glBindVertexArray( 0 );
		glUseProgram( 0 );

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	state.prog = nullptr;



	//TODO: additional cleanup
	
	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}


namespace
{
	void glfw_callback_error_(int aErrNum, char const *aErrDesc)
	{
		std::fprintf(stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum);
	}

	void glfw_callback_key_(GLFWwindow *aWindow, int aKey, int, int aAction, int)
	{
		if (GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction)
		{
			glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
			return;
		}

		if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
		{
			// R-key reloads shaders.
			if (GLFW_KEY_R == aKey && GLFW_PRESS == aAction)
			{
				if (state->prog)
				{
					try
					{
						state->prog->reload();
						std::fprintf(stderr, "Shaders reloaded and recompiled.\n");
					}
					catch (std::exception const &eErr)
					{
						std::fprintf(stderr, "Error when reloading shader:\n");
						std::fprintf(stderr, "%s\n", eErr.what());
						std::fprintf(stderr, "Keeping old shader.\n");
					}
				}
			}

			// Space toggles camera
			if (GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction)
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if (state->camControl.cameraActive)
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				else
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			// Camera controls if camera is active
			if (state->camControl.cameraActive)
			{
				if (GLFW_KEY_W == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveForward = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveForward = false;
				}
				else if (GLFW_KEY_S == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveBackward = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveBackward = false;
				}
				else if (GLFW_KEY_A == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveLeft = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveLeft = false;
				}
				else if (GLFW_KEY_D == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveRight = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveRight = false;
				}
				else if (GLFW_KEY_E == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveUp = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveUp = false;
				}
				else if (GLFW_KEY_Q == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionMoveDown = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionMoveDown = false;
				}
				else if (GLFW_KEY_LEFT_SHIFT == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionSprint = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionSprint = false;
				}
				else if (GLFW_KEY_LEFT_CONTROL == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionCrouch = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionCrouch = false;
				}
					
			}
		}
	}

	void glfw_callback_motion_(GLFWwindow *aWindow, double aX, double aY)
	{
		if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
		{
			if (state->camControl.cameraActive)
			{
				auto const dx = float(aX - state->camControl.lastX);
				auto const dy = float(aY - state->camControl.lastY);

				state->camControl.phi += dx * kMouseSensitivity_;

				state->camControl.theta += dy * kMouseSensitivity_;
				if (state->camControl.theta > kPi_ / 2.f)
					state->camControl.theta = kPi_ / 2.f;
				else if (state->camControl.theta < -kPi_ / 2.f)
					state->camControl.theta = -kPi_ / 2.f;
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
		}
	}
}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if( window )
			glfwDestroyWindow( window );
	}
}
