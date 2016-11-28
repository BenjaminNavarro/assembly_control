/**
 * @file simulator.h
 * @brief Implement a Simulator class to interact with V-REP as well as Signal and MessageBox classes
 * @author Benjamin Navarro
 * @version 1.0.0
 * @date 2015-10-12
 */

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

/**
 * @brief Implementation of a synchronization signal
 */
class Signal
{
public:
	Signal() = default;
	~Signal() = default;


	/**
	 * @brief Wait for the signal to come (blocking call)
	 */
	void wait() {
		std::unique_lock<std::mutex> lock(m_);
		cv_.wait(lock);
	}

	/**
	 * @brief Wait for the signal to come (blocking call) for a certain amount of time
	 * @param ms Number of milliseconds to wait before returning
	 * @return true is the signal has arrived during the timeout period, false otherwise
	 */
	bool wait_for(int ms) {
		std::unique_lock<std::mutex> lock(m_);
		if(cv_.wait_for(lock, std::chrono::milliseconds(ms)) == std::cv_status::timeout)
			return false;
		else
			return true;
	}

	/**
	 * @brief Notify all the waiters that the signal has arrived
	 */
	void notify() {
		cv_.notify_all();
	}

protected:
	std::mutex m_;
	std::condition_variable cv_;

};

/**
 * @brief Implementation of a status signal
 */
class StatusSignal {
private:
	std::mutex m_;
	bool state_;
public:
	StatusSignal() {
		state_ = false;
	}

	/**
	 * @brief Set the signal
	 */
	void set() {
		m_.lock();
		state_ = true;
		m_.unlock();
	}

	/**
	 * @brief Clear the signal
	 */
	void clear() {
		m_.lock();
		state_ = false;
		m_.unlock();
	}

	/**
	 * @brief Check if the signal has already been set.
	 */
	bool is_set() {
		bool ret;

		m_.lock();
		ret = state_;
		m_.unlock();

		return ret;
	}
};


/**
 * @brief Implementation of a message box synchronization system
 *
 * @tparam T Type of the data hold by the message box
 */
template<typename T>
class MessageBox : public Signal
{
public:
	MessageBox() = default;
	~MessageBox() = default;

	/**
	 * @brief Send a new data
	 *
	 * @param data The data to be sent
	 */
	void send(T data) {
		m_.lock();
		data_ = data;
		m_.unlock();
		notify();
	}

	/**
	 * @brief Receive the data (blocking call)
	 *
	 * @return The received data
	 */
	T receive() {
		std::unique_lock<std::mutex> lock(m_);
		cv_.wait(lock);
		return data_;
	}

private:
	T data_;

};

/**
 * @brief Interface with the V-REP simulator
 */
class Simulator
{
public:
	Simulator();
	~Simulator();

	Simulator(const Simulator& sim) {
	}

	Simulator& operator=(const Simulator& sim) {
		return *this;
	}

	/**
	 * @brief Start the simulation in V-REP and the communication thread
	 *
	 * @param cycle_ms Communication thread cycle time (milliseconds)
	 *
	 * @return True if successfully started, false otherwise
	 */
	bool start(int cycle_ms = 25);

	/**
	 * @brief Stop the simulation in V-REP and the communication thread
	 */
	void stop();

	/***	Signals		***/
	/**
	 * @brief Wait for the CO signal (optical barrier)
	 * @param msec number of milliseconds to wait before returning. If negative (default value), the wait is infinite
	 * @return true is the signal has arrived during the timeout period, false otherwise
	 */
	bool wait_co(int msec = -1);
	/**
	 * @brief Wait for the fprise signal (object took by the robot)
	 * @param msec number of milliseconds to wait before returning. If negative (default value), the wait is infinite
	 * @return true is the signal has arrived during the timeout period, false otherwise
	 */
	bool wait_fprise(int msec = -1);
	/**
	 * @brief Wait for the fpose signal (object put down by the robot)
	 * @param msec number of milliseconds to wait before returning. If negative (default value), the wait is infinite
	 * @return true is the signal has arrived during the timeout period, false otherwise
	 */
	bool wait_fpose(int msec = -1);
	/**
	 * @brief Wait for the pos_t1 signal (robot over supply conveyor)
	 * @param msec number of milliseconds to wait before returning. If negative (default value), the wait is infinite
	 * @return true is the signal has arrived during the timeout period, false otherwise
	 */
	bool wait_pos_t1(int msec = -1);
	/**
	 * @brief Wait for the pos_t2 signal (robot over evacuation conveyor)
	 * @param msec number of milliseconds to wait before returning. If negative (default value), the wait is infinite
	 * @return true is the signal has arrived during the timeout period, false otherwise
	 */
	bool wait_pos_t2(int msec = -1);
	/**
	 * @brief Wait for the pos_assem signal (robot over assembly station)
	 * @param msec number of milliseconds to wait before returning. If negative (default value), the wait is infinite
	 * @return true is the signal has arrived during the timeout period, false otherwise
	 */
	bool wait_pos_assem(int msec = -1);
	/**
	 * @brief Wait for the arret_t2 signal (evacuation conveyor stopped)
	 * @param msec number of milliseconds to wait before returning. If negative (default value), the wait is infinite
	 * @return true is the signal has arrived during the timeout period, false otherwise
	 */
	bool wait_arret_t2(int msec = -1);

	/**
	 * @brief Read the fin_reccam signal (end of object recognition)
	 *
	 * @return State of the signal
	 */
	bool read_fin_reccam();
	/**
	 * @brief Read the p1 signal (type 1 object)
	 *
	 * @return State of the signal
	 */
	bool read_p1();
	/**
	 * @brief Read the p2 signal (type 2 object)
	 *
	 * @return State of the signal
	 */
	bool read_p2();
	/**
	 * @brief Read the p3 signal (type 3 object)
	 *
	 * @return State of the signal
	 */
	bool read_p3();
	/**
	 * @brief Read the fin_OP1 signal (assembly operation 1 finished)
	 *
	 * @return State of the signal
	 */
	bool read_fin_OP1();
	/**
	 * @brief Read the fin_OP2 signal (assembly operation 2 finished)
	 *
	 * @return State of the signal
	 */
	bool read_fin_OP2();
	/**
	 * @brief Read the fin_OP3 signal (assembly operation 3 finished)
	 *
	 * @return State of the signal
	 */
	bool read_fin_OP3();
	/**
	 * @brief Read the assemblage_conforme signal (good assembly)
	 *
	 * @return State of the signal
	 */
	bool read_assemblage_conforme();
	/**
	 * @brief Read the assemblage_evacue signal (evacuated assembly)
	 *
	 * @return State of the signal
	 */
	bool read_assemblage_evacue();

	/***	Commands	***/
	/**
	 * @brief Set the AV_T1 command (supply conveyor)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	void set_AV_T1(bool state);
	/**
	 * @brief Set the AV_T2 command (evacuation conveyor)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	void set_AV_T2(bool state);
	/**
	 * @brief Set the Reccam command (object recognition)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	void set_Reccam(bool state);
	/**
	 * @brief Set the D command (robot going to the right)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	void set_D(bool state);
	/**
	 * @brief Set the G command (robot going to the left)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	void set_G(bool state);
	/**
	 * @brief Set the Prend command (take an object)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	virtual void set_Prend(bool state);
	/**
	 * @brief Set the Pose command (put down an object)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	virtual void set_Pose(bool state);
	/**
	 * @brief Set the OP1 command (assembly operation 1)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	void set_OP1(bool state);
	/**
	 * @brief Set the OP2 command (assembly operation 2)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	void set_OP2(bool state);
	/**
	 * @brief Set the OP3 command (assembly operation 3)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	void set_OP3(bool state);
	/**
	 * @brief Set the Verif command (assembly verification)
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	void set_Verif(bool state);

protected:
	/**
	 * @brief Get object handles from V-REP
	 *
	 * @return True if successful, false otherwise
	 */
	bool get_Handles();
	/**
	 * @brief Start data streaming from V-REP
	 *
	 * @return True if successful, false otherwise
	 */
	bool start_Streaming();
	/**
	 * @brief Get the current time since epoch
	 *
	 * @return Time in seconds (us precision)
	 */
	double get_Current_Time();

	/**
	 * @brief Communication thread
	 *
	 * @param cycle_ms Cycle time (milliseconds)
	 */
	void process(int cycle_ms);


	/**
	 * @brief Commands sent to V-REP
	 */
	struct commands_t
	{
		bool AV_T1;
		bool AV_T2;
		bool Reccam;
		bool D;
		bool G;
		bool Prend;
		bool Pose;
		bool OP1;
		bool OP2;
		bool OP3;
		bool Verif;
	};
	commands_t commands_;


	/**
	 * @brief Signals read from V-REP
	 */
	struct signals_t
	{
		Signal co;
		Signal fprise;
		Signal fpose;
		Signal pos_t1;
		Signal pos_t2;
		Signal pos_assem;
		Signal arret_t2;

		bool fin_reccam;
		bool p1;
		bool p2;
		bool p3;
		bool scg;
		bool scd;
		bool fin_OP1;
		bool fin_OP2;
		bool fin_OP3;
		bool assemblage_conforme;
		bool assemblage_evacue;
	};
	signals_t signals_;

	/**
	 * @brief Current robot position
	 */
	enum Position {
		PosAssembly = 1,
		PosAppro = 2,
		PosEvac = 3,
	};
	Position prev_position_;

	int prev_gripper_state_;
	int prev_optical_barrier_state_;
	int prev_evac_conveyor_state_;

	double last_created_object_time_;
	int last_created_object_type_;

	std::thread thread_;
	bool run_;

	int client_id_;
	int appro_prox_sensor_handle_;

};

extern Simulator sim;
#endif
