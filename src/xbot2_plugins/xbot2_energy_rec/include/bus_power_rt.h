#ifndef BUS_POWER_RT_H
#define BUS_POWER_RT_H

#include <matlogger2/matlogger2.h>

#include <xbot2/xbot2.h>

#include <cartesian_interface/CartesianInterfaceImpl.h>

#include <thread>

#include <awesome_utils/awesome_utils/calib_utils.hpp>
#include <awesome_utils/awesome_utils/sign_proc_utils.hpp>
#include <awesome_utils/xbot2_utils/xbot2_utils.hpp>
#include <awesome_utils/awesome_utils/power_utils.hpp>

#include <std_msgs/Bool.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <xbot2/hal/dev_ft.h>

#if defined(USE_ECAT_COMPONENTS)
    #include <ec_xbot2/pow_ec.h>
#endif

#include <xbot2/ros/ros_support.h>
#include <xbot_msgs/CustomState.h>
#include <xbot_msgs/JointState.h>

#include <cartesian_interface/ros/RosServerClass.h>

#include <cartesian_interface/sdk/rt/LockfreeBufferImpl.h>

#include <awesome_leg/IqEstStatus.h>
#include <awesome_leg/EstRegPowStatus.h>
#include <awesome_leg/MeasRegPowStatus.h>
#include <awesome_leg/IqMeasStatus.h>

#include <awesome_leg/SetRegEnergyMonitoringStatus.h>

using namespace XBot;
using namespace CalibUtils;
using namespace SignProcUtils;
using namespace Xbot2Utils;
using namespace PowerUtils;

/**
 * @brief The BusPowerRt class is a rt class to validate
 * the power and energy which flows from/to the power bus stage
 * of a system of FOC BLDC actuators
**/

class BusPowerRt : public ControlPlugin
{

public:

    using ControlPlugin::ControlPlugin;

    // initialization method; the plugin won't be run
    // if this returns 'false'
    bool on_initialize() override;

    // callback for the 'Starting' state
    // start_completed() must be called to switch
    // to 'Run' state
    void starting() override;

    // callback for 'Run' state
    void run() override;

    // callback for 'On Stop' state
    void stopping() override;

    // callback for 'On Stop' state
    void on_stop() override;

    // callback for 'On Abort' state
    void on_close() override;

    // callback for 'On Abort' state
    void on_abort() override;

private:

    bool _is_sim = true, _is_dummy = false,
         _use_iq_meas = false,
         _dump_iq_data = false,
         _monitor_recov_energy = false, 
         _verbose = false,
         _power_sensor_found = false,
         _enable_meas_rec_energy_monitoring = false;

    int _n_jnts_robot,
        _n_jnts_model,
        _der_est_order = 1,
        _iq_calib_window_size = 1000,
        _alpha = 5.0;

    std::string _mat_path = "/tmp/", _dump_mat_suffix = "bus_power_rt",
                _hw_type, 
                _topic_ns = "",
                _set_monitor_state_servname = "start_rec_energy_monitor",
                _urdf_path, _srdf_path;

    double _plugin_dt,
        _loop_time = 0.0, _loop_timer_reset_time = 3600.0,
        _matlogger_buffer_size = 1e5,
        _lambda_qp_reg = 1.0,
        _q_dot_3sigma = 0.01;

    double _er = 0.0, _pr = 0.0, _recov_energy_tot = 0.0;

    // power sensor
    double _vbatt = 0.0, _ibatt = 0.0, _e_batt = 0.0, _p_batt = 0.0, _reg_energy = 0.0;
    double _bus_p_leak = 0.0;

    Eigen::VectorXd _q_p_meas,
                    _q_p_dot_meas, _q_dot_motor, _q_p_dot_meas_filt, _q_p_ddot_est, _q_p_ddot_est_filt,
                    _q_model, _v_model, _tau_model, _v_dot_model,
                    _tau_meas, _tau_meas_filt,
                    _iq_meas, _iq_meas_filt,
                    _K_t, _K_d0, _K_d1, _rot_MoI, _red_ratio,
                    _tau,
                    _iq_est, _iq_friction_torque, _tau_rot_est,
                    _alpha_f0, _alpha_f1,
                    _R, _L_leak, _L_m,
                    _jnt_stiffness_setpoint, _jnt_damping_setpoint,
                    _q_ref, _q_dot_ref;

    Eigen::VectorXd _er_k, _pr_k, _recov_energy,
                    _pk_joule, _pk_mech, _pk_indct,
                    _ek_joule, _ek_mech, _ek_indct;

    Eigen::Matrix6d _Lambda_inv; // used for computing the "impact severity ratio"
    Eigen::MatrixXd _Jc, _B, _Lambda_inv_tmp;
    double _rest_coeff = 0.0;
    double _imp_severity_ratio = -1.0;

    std::vector<double> _iq_est_vect, _q_p_ddot_est_vect, _q_p_ddot_est_filt_vect,
                        _q_p_dot_meas_vect, _q_p_dot_meas_filt_vect,
                        _tau_meas_vect, _tau_meas_filt_vect,
                        _K_t_vect,
                        _K_d0_vect, _K_d1_vect,
                        _rot_MoI_vect, _red_ratio_vect, _iq_friction_torque_vect,
                        _iq_friction_torque_cal_vect,
                        _tau_rot_est_vect,
                        _alpha_f0_vect, _alpha_f1_vect,
                        _K_d0_cal_vect, _K_d1_cal_vect,
                        _iq_meas_vect,
                        _iq_meas_filt_vect;

    std::vector<double> _er_k_vect, _pr_k_vect, _recov_energy_vect,
                        _pk_joule_vect, _pk_mech_vect, _pk_indct_vect,
                        _ek_joule_vect, _ek_mech_vect, _ek_indct_vect;

    std::vector<double> _q_model_vect, _stiffness_setpoint_vect, _damping_setpoint_vect,
                        _q_ref_vect, _q_dot_ref_vect;

    std::vector<std::string> _jnt_names, _iq_jnt_names;

    MatLogger2::Ptr _dump_logger;

    // handle adapting ROS primitives for RT support
    RosSupport::UniquePtr _ros;

    // queue object to handle multiple subscribers/servers at once
    CallbackQueue _queue;

    SubscriberPtr<xbot_msgs::CustomState> _aux_signals_sub;
    SubscriberPtr<xbot_msgs::JointState> _js_signals_sub;

    PublisherPtr<awesome_leg::IqEstStatus> _iq_est_pub;
    PublisherPtr<awesome_leg::EstRegPowStatus> _est_reg_pow_pub;
    PublisherPtr<awesome_leg::MeasRegPowStatus> _meas_reg_pow_pub;
    PublisherPtr<awesome_leg::IqMeasStatus> _iq_meas_pub;

    ServiceServerPtr<awesome_leg::SetRegEnergyMonitoringStatusRequest, awesome_leg::SetRegEnergyMonitoringStatusResponse> _set_monitoring_state_srvr;

    IqRosGetter::Ptr _iq_getter;
    IqEstimator::Ptr _iq_estimator;

    RegEnergy::Ptr _pow_estimator;
    #if defined(USE_ECAT_COMPONENTS)
    std::shared_ptr<Hal::PowerBoardEc> _pow_sensor;
    #endif
    NumIntRt _meas_pow_int, _reg_meas_pow_int;
    Eigen::VectorXd _dummy_eig_scalar; // aux variable

    NumDiff _num_diff;

    MovAvrgFilt _mov_avrg_filter_iq;
    MovAvrgFilt _mov_avrg_filter_tau;
    MovAvrgFilt _mov_avrg_filter_q_dot;

    int _mov_avrg_window_size_iq = 10;
    double _mov_avrg_cutoff_freq_iq = 15.0;
    int _mov_avrg_window_size_tau = 10;
    double _mov_avrg_cutoff_freq_tau = 15.0;
    int _mov_avrg_window_size_iq_meas = 10;
    double _mov_avrg_cutoff_freq_iq_meas = 15.0;
    int _mov_avrg_window_size_q_dot= 10;
    double _mov_avrg_cutoff_freq_q_dot = 15.0;

    XBot::ModelInterface::Ptr _model;

    void get_params_from_config();

    void init_vars();
    void init_clocks();
    void init_dump_logger();

    void reset_flags();

    void update_state();
    void update_sensed_power();
    void update_clocks();

    void add_data2dump_logger();
    void add_data2bedumped();

    void spawn_nrt_thread();

    void init_nrt_ros_bridge();
    void init_model_interface();

    void is_sim(std::string sim_string);
    void is_dummy(std::string dummy_string);

    void pub_iq_est();
    void pub_est_reg_pow();
    void pub_meas_reg_pow();
    void pub_iq_meas();

    void run_iq_estimation();
    void run_reg_pow_estimation();

    bool on_monitor_state_signal(const awesome_leg::SetRegEnergyMonitoringStatusRequest& req, awesome_leg::SetRegEnergyMonitoringStatusResponse& res);

};

#endif // BUS_POWER_RT_H
