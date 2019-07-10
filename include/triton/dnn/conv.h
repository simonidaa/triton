#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include "triton/driver/stream.h"
#include "triton/driver/kernel.h"
#include "triton/dnn/base.h"

namespace triton{
namespace dnn{

class conv: public base{
public:
  enum type {
    FPROP,
    BPROP,
    WGRAD
  };

private:
  // initialize
  std::tuple<int32_t, int32_t, int32_t, int32_t>
      unpack(int32_t ltrs, bool flip, int32_t EBD, int32_t EBH, int32_t EBW);
  void build_b_deltas();
  void build_a_deltas();
  void build_masks();
  void init_impl(driver::stream *, driver::cu_module *);

  // enqueue
  std::array<size_t, 3> get_grid(size_t TM, size_t TN);
  void set_arg(driver::kernel *kernel,
               driver::buffer *a, driver::buffer *b, driver::buffer *c,
               driver::buffer *bias);
  void enqueue_impl(driver::stream *stream, driver::kernel *kernel,
                    std::vector<driver::buffer*> args,
                    const std::vector<unsigned>& ranges,
                    size_t nthreads);
  // number of flops
  size_t num_flops() const;
  // comparison for maps
  bool operator<(const base& other) const;
  // clone
  base* clone() const;

public:

  conv(int B, int NC,
       int D, int H, int W,
       int T, int R, int S, int NF,
       int stride_d, int stride_h, int stride_w,
       int pad_d, int pad_h, int pad_w,
       int upsample_d, int upsample_h, int upsample_w,
       std::string a_ty = "fp32", std::string b_ty = "fp32",
       type ty = FPROP, bool bias = false);

  // accessors
  size_t a_size();
  size_t b_size();
  size_t c_size();
  std::vector<int32_t> c_shapes();
  // default params
  std::vector<unsigned> default_params();

  // triton-c source code
  void triton_c_src(std::ostream &os) const;

  // cpu reference implementations
  template<class IN_DTYPE, class OUT_DTYPE>
  void cpu_xprop(OUT_DTYPE* C,  IN_DTYPE* A, IN_DTYPE* B);
  template<class IN_DTYPE, class OUT_DTYPE>
  void cpu_wgrad(OUT_DTYPE* C,  IN_DTYPE* A, IN_DTYPE* B);
  template<class IN_DTYPE, class OUT_DTYPE>
  void cpu_ref(OUT_DTYPE* C,  IN_DTYPE* A, IN_DTYPE* B);

private:
  // image size
  int32_t NB_;
  int32_t NC_;
  int32_t AD_;
  int32_t AH_;
  int32_t AW_;
  // filter size
  int32_t BD_;
  int32_t BH_;
  int32_t BW_;
  int32_t NF_;
  // activation size
  int32_t CD_;
  int32_t CH_;
  int32_t CW_;
  // striding
  int32_t stride_d_;
  int32_t stride_h_;
  int32_t stride_w_;
  // padding
  int32_t pad_d_;
  int32_t pad_h_;
  int32_t pad_w_;
  // upsampling
  int32_t upsample_d_;
  int32_t upsample_h_;
  int32_t upsample_w_;
  // equivalent matmul
  int32_t M_;
  int32_t N_;
  int32_t K_;
  // helpers
  int32_t Fs_;
  int32_t TK_;
  int32_t Luts_;
  // memory strides for A
  std::vector<int32_t> shapes_a_;
  std::vector<int32_t> ld_a_;
  // memory strides for B
  std::vector<int32_t> shapes_b_;
  std::vector<int32_t> ld_b_;
  // memory stride for C
  std::vector<int32_t> shapes_c_;
  std::vector<int32_t> ld_c_;
  // constant memory
  std::vector<int32_t> h_a_deltas_;
  std::vector<int32_t> h_b_deltas_;
  std::vector<int32_t> h_masks_;
  driver::buffer* d_a_deltas_;
  driver::buffer* d_b_deltas_;
  driver::buffer* d_masks_;
  driver::buffer* d_locks_;
  bool is_a_deltas_cst;
  bool is_b_deltas_cst_;
  bool is_mask_cst_;
  // data type
  std::string a_ty_;
  std::string b_ty_;
  // conv type
  type ty_;
  bool bias_;
  bool b_trans_;
  bool b_lut_;
  // axis index
  int32_t a_inner_idx_;
  int32_t a_outer_idx_;
  int32_t a_pix_idx_;
  int32_t b_inner_idx_;
  int32_t b_outer_idx_;
  int32_t b_pix_idx_;
  int32_t c_outer_0_idx_;
  int32_t c_outer_1_idx_;
  int32_t c_pix_idx;
  // maximum grid size for loc
  int32_t max_grid_0_;
  int32_t max_grid_1_;
};

}
}
