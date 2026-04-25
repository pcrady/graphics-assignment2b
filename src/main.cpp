// This template code was originally written by Matt Overby while a TA for CSci5607

// The loaders are included by glfw3 (glcorearb.h) if we are not using glew.
#include "glad/glad.h"

#include "GLFW/glfw3.h"

// Includes
#include "shader.hpp"
#include "trimesh.hpp"
#include <cmath>
#include <cstring> // memcpy
// Constants
#define WIN_WIDTH 500
#define WIN_HEIGHT 500

enum class KeyState {
  NONE,
  MOVE_FORWARD,
  MOVE_BACKWARD,
  MOVE_LEFT,
  MOVE_RIGHT,
  LOOK_LEFT,
  LOOK_RIGHT
};

class Mat4x4 {
public:
  float m[16];

  /*
  void make_identity(){
          m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
          m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
          m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
          m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
  }
  */
  Mat4x4() { // Default: Identity
    m[0] = 1.f;
    m[4] = 0.f;
    m[8] = 0.f;
    m[12] = 0.f;
    m[1] = 0.f;
    m[5] = 1.f;
    m[9] = 0.f;
    m[13] = 0.f;
    m[2] = 0.f;
    m[6] = 0.f;
    m[10] = 1.f;
    m[14] = 0.f;
    m[3] = 0.f;
    m[7] = 0.f;
    m[11] = 0.f;
    m[15] = 1.f;
  }

  void make_identity() {
    m[0] = 1.f;
    m[4] = 0.f;
    m[8] = 0.f;
    m[12] = 0.f;
    m[1] = 0.f;
    m[5] = 1.f;
    m[9] = 0.f;
    m[13] = 0.f;
    m[2] = 0.f;
    m[6] = 0.f;
    m[10] = 1.f;
    m[14] = 0.f;
    m[3] = 0.f;
    m[7] = 0.f;
    m[11] = 0.f;
    m[15] = 1.f;
  }

  void print() {
    std::cout << m[0] << ' ' << m[4] << ' ' << m[8] << ' ' << m[12] << "\n";
    std::cout << m[1] << ' ' << m[5] << ' ' << m[9] << ' ' << m[13] << "\n";
    std::cout << m[2] << ' ' << m[6] << ' ' << m[10] << ' ' << m[14] << "\n";
    std::cout << m[3] << ' ' << m[7] << ' ' << m[11] << ' ' << m[15] << "\n";
  }

  void make_scale(float x, float y, float z) {
    make_identity();
    m[0] = x;
    m[5] = y;
    m[10] = z;
  }
};

static inline const Vec3f operator*(const Mat4x4& m, const Vec3f& v) {
  Vec3f r(m.m[0] * v[0] + m.m[4] * v[1] + m.m[8] * v[2], m.m[1] * v[0] + m.m[5] * v[1] + m.m[9] * v[2], m.m[2] * v[0] + m.m[6] * v[1] + m.m[10] * v[2]);
  return r;
}

static inline const Vec3f operator+(const Vec3f& u, const Vec3f& v) {
  Vec3f r(
    u[0] + v[0],
    u[1] + v[1],
    u[2] + v[2]
  );
  return r;
}

static inline const Vec3f operator-(const Vec3f& u, const Vec3f& v) {
  Vec3f r(
    u[0] - v[0],
    u[1] - v[1],
    u[2] - v[2]
  );
  return r;
}

static inline const float dot_product(const Vec3f& u, const Vec3f& v) {
  return u[0] * v[0] +
         u[1] * v[1] +
         u[2] * v[2];
}

static inline const Vec3f cross_product(const Vec3f& u, const Vec3f& v) {
  Vec3f r(
    u[1] * v[2] - u[2] * v[1],
    u[2] * v[0] - u[0] * v[2],
    u[0] * v[1] - u[1] * v[0]
  );
  return r;
}

static inline const float magnitude(const Vec3f& v) {
  return sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
}

static inline const Vec3f normalize(const Vec3f& v) {
  float mag = magnitude(v);

  if (mag == 0) {
    mag = 0.0001;
  }

  Vec3f r(
    v[0] / mag,
    v[1] / mag,
    v[2] / mag
  );
  return r;
}


static inline Mat4x4 multiplyMatrices(const GLfloat A[16], const GLfloat B[16]) {
  Mat4x4 result;
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      result.m[col * 4 + row] =
        A[0 * 4 + row] * B[col * 4 + 0] +
        A[1 * 4 + row] * B[col * 4 + 1] +
        A[2 * 4 + row] * B[col * 4 + 2] +
        A[3 * 4 + row] * B[col * 4 + 3];
    }
  }
  return result;
}


//
//	Global state variables
//
namespace Globals {
enum KeyState key_state = KeyState::NONE;
double cursorX, cursorY;     // cursor positions
float win_width, win_height; // window size
float aspect;
GLuint verts_vbo[1], colors_vbo[1], normals_vbo[1], faces_ibo[1], tris_vao;
TriMesh mesh;

//  Model, view and projection matrices, initialized to the identity
Mat4x4 model;
Mat4x4 view;
Mat4x4 projection;
Mat4x4 view_rotation;

GLdouble ds = 0.1;
GLdouble dtheta = 0.01;
Vec3f up(0, 1, 0);
} // namespace Globals


//
//	Callbacks
//
static void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  // Close on escape or Q
  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, GL_TRUE);
      break;
    case GLFW_KEY_Q:
      glfwSetWindowShouldClose(window, GL_TRUE);
      break;

    case GLFW_KEY_W:
      Globals::key_state = KeyState::MOVE_FORWARD;
      break;
    case GLFW_KEY_S:
      Globals::key_state = KeyState::MOVE_BACKWARD;
      break;
    case GLFW_KEY_A:
      Globals::key_state = KeyState::MOVE_LEFT;
      break;
    case GLFW_KEY_D:
      Globals::key_state = KeyState::MOVE_RIGHT;
      break;
    case GLFW_KEY_LEFT:
      Globals::key_state = KeyState::LOOK_LEFT;
      break;
    case GLFW_KEY_RIGHT:
      Globals::key_state = KeyState::LOOK_RIGHT;
      break;
    }
  }

  if (action == GLFW_RELEASE) {
    Globals::key_state = KeyState::NONE;
  }
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  int win_width, win_height;
  float aspect;
  glfwGetFramebufferSize(window, &win_width, &win_height);

  aspect = (float)win_width / (float)win_height;

  glViewport(0, 0, width, height);

  // ToDo: update the perspective matrix as the window size changes
}


// Function to set up geometry
void init_scene();


static void update_view(Vec3f& eye, Vec3f& lookat) {
  Vec3f w = normalize(Globals::view_rotation * lookat);
  Vec3f u = normalize(cross_product(Globals::up, w));
  Vec3f v = normalize(cross_product(w, u));

  float dx = -dot_product(eye, u);
  float dy = -dot_product(eye, v);
  float dz = -dot_product(eye, w);

  Globals::view.m[0] = u[0];
  Globals::view.m[1] = v[0];
  Globals::view.m[2] = w[0];
  Globals::view.m[3] = 0;

  Globals::view.m[4] = u[1];
  Globals::view.m[5] = v[1];
  Globals::view.m[6] = w[1];
  Globals::view.m[7] = 0;

  Globals::view.m[8] = u[2];
  Globals::view.m[9] = v[2];
  Globals::view.m[10] = w[2];
  Globals::view.m[11] = 0;

  Globals::view.m[12] = dx;
  Globals::view.m[13] = dy;
  Globals::view.m[14] = dz;
  Globals::view.m[15] = 1;
}


static void init_projection() {
  float left = -0.1;
  float right = 0.1;
  float bottom = -0.1;
  float top = 0.1;
  float near = 0.1;
  float far = 1000;

  Globals::projection.m[0] = (2 * near) / (left - right);
  Globals::projection.m[5] = (2 * near) / (top - bottom);
  Globals::projection.m[8] = (right + left) / (right - left);
  Globals::projection.m[9] = (top + bottom) / (top - bottom);
  Globals::projection.m[10] = -(far + near) / (far - near);
  Globals::projection.m[11] = -1;
  Globals::projection.m[14] = (-2 * far * near) / (far - near);
}


//
//	Main
//
int main(int argc, char* argv[]) {

  // Load the mesh
  std::stringstream obj_file;
  obj_file << MY_DATA_DIR << "sibenik/sibenik.obj";
  if (!Globals::mesh.load_obj(obj_file.str())) {
    return 0;
  }
  Globals::mesh.print_details();


  // Set up the window variable
  GLFWwindow* window;

  // Define the error callback function
  glfwSetErrorCallback(&error_callback);

  // Initialize glfw
  if (!glfwInit()) {
    return EXIT_FAILURE;
  }

  // Ask for OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Create the glfw window
  Globals::win_width = WIN_WIDTH;
  Globals::win_height = WIN_HEIGHT;
  window = glfwCreateWindow(int(Globals::win_width), int(Globals::win_height), "HW2b", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return EXIT_FAILURE;
  }

  // Define callbacks to handle user input and window resizing
  glfwSetKeyCallback(window, &key_callback);
  glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

  // More setup stuff
  glfwMakeContextCurrent(window); // Make the window current
  glfwSwapInterval(1);            // Set the swap interval

  // make sure the openGL code can be found; folks using Windows need this
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to gladLoadGLLoader" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  // Initialize the shaders
  // MY_SRC_DIR was defined in CMakeLists.txt
  // it specifies the full path to this project's src/ directory.
  mcl::Shader shader;
  std::stringstream ss;
  ss << MY_SRC_DIR << "shader.";
  shader.init_from_files(ss.str() + "vert", ss.str() + "frag");

  // Initialize the scene
  init_scene();
  framebuffer_size_callback(window, int(Globals::win_width), int(Globals::win_height));

  // Perform some OpenGL initializations
  glEnable(GL_DEPTH_TEST);          // turn hidden surfce removal on
  glClearColor(1.f, 1.f, 1.f, 1.f); // set the background to white

  // Enable the shader, this allows us to set uniforms and attributes
  shader.enable();

  // Bind buffers
  glBindVertexArray(Globals::tris_vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Globals::faces_ibo[0]);

  Vec3f eye(0, 0, 0);
  Vec3f lookat(0, 0, -1);
  float rotation_angle = 0;

  update_view(eye, lookat);
  init_projection();

  // Game loop
  while (!glfwWindowShouldClose(window)) {

    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch (Globals::key_state) {
    case KeyState::NONE:
      break;
    case KeyState::MOVE_FORWARD:
      eye[2] = eye[2] + Globals::ds;
      update_view(eye, lookat);
      Globals::view.print();
      printf("\n");
      break;
    case KeyState::MOVE_BACKWARD:
      eye[2] = eye[2] - Globals::ds;
      update_view(eye, lookat);
      Globals::view.print();
      printf("\n");
      break;
    case KeyState::MOVE_LEFT:
      eye[0] = eye[0] - Globals::ds;
      update_view(eye, lookat);
      Globals::view.print();
      printf("\n");
      break;
    case KeyState::MOVE_RIGHT:
      eye[0] = eye[0] + Globals::ds;
      update_view(eye, lookat);
      Globals::view.print();
      printf("\n");
      break;

    case KeyState::LOOK_LEFT:
      rotation_angle = rotation_angle - Globals::dtheta;
      Globals::view_rotation.m[0] = std::cos(rotation_angle);
      Globals::view_rotation.m[2] = -std::sin(rotation_angle);
      Globals::view_rotation.m[8] = std::sin(rotation_angle);
      Globals::view_rotation.m[10] = std::cos(rotation_angle);
      update_view(eye, lookat);
      Globals::view.print();
      printf("\n");
      break;

    case KeyState::LOOK_RIGHT:
      rotation_angle = rotation_angle + Globals::dtheta;
      Globals::view_rotation.m[0] = std::cos(rotation_angle);
      Globals::view_rotation.m[2] = -std::sin(rotation_angle);
      Globals::view_rotation.m[8] = std::sin(rotation_angle);
      Globals::view_rotation.m[10] = std::cos(rotation_angle);
      update_view(eye, lookat);
      Globals::view.print();
      printf("\n");
      break;
    }

    // Globals::view.print();
    // std::cout << "rotation angle: " << rotation_angle << "\n";
    // printf("\n");


    // Send updated info to the GPU
    glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, Globals::model.m);           // model transformation
    glUniformMatrix4fv(shader.uniform("view"), 1, GL_FALSE, Globals::view.m);             // viewing transformation
    glUniformMatrix4fv(shader.uniform("projection"), 1, GL_FALSE, Globals::projection.m); // projection matrix
                                                                                          //
    // Draw
    glDrawElements(GL_TRIANGLES, Globals::mesh.faces.size() * 3, GL_UNSIGNED_INT, 0);

    // Finalize
    glfwSwapBuffers(window);
    glfwPollEvents();

  } // end game loop

  // Unbind
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Disable the shader, we're done using it
  shader.disable();

  return EXIT_SUCCESS;
}


void init_scene() {

  using namespace Globals;

  // Create the buffer for vertices
  glGenBuffers(1, verts_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(mesh.vertices[0]), &mesh.vertices[0][0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Create the buffer for colors
  glGenBuffers(1, colors_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, mesh.colors.size() * sizeof(mesh.colors[0]), &mesh.colors[0][0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Create the buffer for normals
  glGenBuffers(1, normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(mesh.normals[0]), &mesh.normals[0][0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Create the buffer for indices
  glGenBuffers(1, faces_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ibo[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.faces.size() * sizeof(mesh.faces[0]), &mesh.faces[0][0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Create the VAO
  glGenVertexArrays(1, &tris_vao);
  glBindVertexArray(tris_vao);

  int vert_dim = 3;

  // location=0 is the vertex
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
  glVertexAttribPointer(0, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.vertices[0]), 0);

  // location=1 is the color
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
  glVertexAttribPointer(1, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.colors[0]), 0);

  // location=2 is the normal
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
  glVertexAttribPointer(2, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.normals[0]), 0);

  // Done setting data for the vao
  glBindVertexArray(0);
}
