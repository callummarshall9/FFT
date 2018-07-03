#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
   #include "ffft/FFTReal.h"


#define SAMPLE_COUNT 1024
float output_stream[SAMPLE_COUNT];
ffft::FFTReal <double> fft_object (SAMPLE_COUNT * 2);

// custom audio stream that plays a loaded buffer
class MyStream : public sf::SoundStream
{
public:

    void load(const sf::SoundBuffer& buffer)
    {
        // extract the audio samples from the sound buffer to our own container
        m_samples.assign(buffer.getSamples(), buffer.getSamples() + buffer.getSampleCount());

        // reset the current playing position
        m_currentSample = 0;

        // initialize the base class
        initialize(buffer.getChannelCount(), buffer.getSampleRate());
    }

private:

    virtual bool onGetData(Chunk& data)
    {
        // number of samples to stream every time the function is called;
        // in a more robust implementation, it should be a fixed
        // amount of time rather than an arbitrary number of samples
        const int samplesToStream = SAMPLE_COUNT * 2;

        // set the pointer to the next audio samples to be played
        data.samples = &m_samples[m_currentSample];
        // have we reached the end of the sound?

				double f[samplesToStream];
				double f_i[samplesToStream];
				double x[samplesToStream];
				for(int i = 0; i < samplesToStream; i++) {
					x[i] = data.samples[i];
				}
				fft_object.do_fft(f,x);
				fft_object.do_ifft(x,f_i);
				fft_object.rescale (x);
				for(int i = 0; i < SAMPLE_COUNT; i++) {
					output_stream[i] = sqrt(f[i]*f[i] + f_i[i] * f_i[i]);
				}

        if (m_currentSample + samplesToStream <= m_samples.size())
        {
            // end not reached: stream the samples and continue
            data.sampleCount = samplesToStream;
            m_currentSample += samplesToStream;
            return true;
        }
        else
        {
            // end of stream reached: stream the remaining samples and stop playback
            data.sampleCount = m_samples.size() - m_currentSample;
            m_currentSample = m_samples.size();
            return false;
        }
    }

    virtual void onSeek(sf::Time timeOffset)
    {
        // compute the corresponding sample index according to the sample rate and channel count
        m_currentSample = static_cast<std::size_t>(timeOffset.asSeconds() * getSampleRate() * getChannelCount());
    }

    std::vector<sf::Int16> m_samples;
    std::size_t m_currentSample;
};

int main()
{
    // load an audio buffer from a sound file
    sf::SoundBuffer buffer;
    buffer.loadFromFile("output.wav");

    // initialize and play our custom stream
    MyStream stream;
    stream.load(buffer);
    stream.play();
		int window_height = 800;
		sf::RenderWindow window(sf::VideoMode(1024, window_height), "FFT audio visualiser");

		while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw everything here...
        // window.draw(...);
				for(int i = 0; i < SAMPLE_COUNT; i++) {
					double height = output_stream[i] / 10000;
					sf::RectangleShape rectangle(sf::Vector2f(1, height));
					rectangle.setFillColor(sf::Color(100, 250, 50));
					rectangle.setPosition(i, window_height-height);
					window.draw(rectangle);
				}

        // end the current frame
        window.display();
    }

    return 0;
}
