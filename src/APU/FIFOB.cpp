#include "FIFOB.hpp"
#include "Bus/IO/IOContainer.hpp"
#include "CPU/ARM7TDMI.hpp"

int16 FIFOB::generate_sample() {
	if(m_audio_samples.empty()) {
		return 0;
	}

	const int16 sample = static_cast<int16>(static_cast<int8>(m_audio_samples.front())) / (2 - io().soundctlH->volumeB);
	m_audio_samples.pop_front();

	return sample;
}

void FIFOB::push_sample(uint8 sample, unsigned int sample_rate) {
	if(sample_rate == 0) {
		m_audio_samples.push_back(sample);
		return;
	}
	for(unsigned i = 0; i < 262144 / sample_rate; ++i) {
		m_audio_samples.push_back(sample);
	}
}

void FIFOB::push_raw(uint8 sample) {
	m_raw_queue.push_back(sample);
}

void FIFOB::move_data_to_sound_circuit(unsigned int sample_rate) {
	//  If the channel isn't enabled
	if(!(io().soundctlH->enable_left_B || io().soundctlH->enable_right_B)) {
		return;
	}

	if(!m_raw_queue.empty()) {
		//  Output sample to audio engine
		push_sample(m_raw_queue.front(), sample_rate);
		m_raw_queue.pop_front();
	}

	//  Check if FIFOs contain enough data
	if(m_raw_queue.size() == 16 || m_raw_queue.empty()) {
		cpu().dma_request_fifoB();
	}
}

void FIFOB::clear_raw() {
	m_raw_queue.clear();
}
