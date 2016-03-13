

    /**
     * Interface describing a drawable 2D object.
     * 
     * Any object implementing this interface can be drawn using a Plotter2D.
     * 
     * @warning : the public methods of the interface must be thread-safe !
     **/
    class Drawable2DInterface
    {

        public:

        /** Default constructor. */
        Drawable2DInterface() {}

        /** virtual Destructor. */
        virtual ~Drawable2DInterface() {}


        /**
         * Sets the parameters for the drawing.
         *
         * @param   range       The range to draw.
         * @param   imageSize   Size of the desired picture.
         **/
        virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) = 0;


        /**
         * Request a reset of the drawing. This method is called to indicate that the underlying object
         * drawn may have changed, previous drawing should be discarded and redrawn.
         **/
        virtual void resetDrawing() { if (useThreads() { MTOOLS_ERROR("resetDrawing should be overriden."); } return; }


        /**
         * Draw onto a given image. This method is called when we want the picture. This method should
         * be as fast as possible  (eventually indicating that it is incomplete by returning a number
         * smaller than 100).
         *
         * @param [in,out]  im  The image to draw onto.
         * @param   opacity     The opacity that should be applied to the picture before drawing onto im.
         *                      Hence, if opacity = 1.0, overwrite im and if opacity = 0.0 do nothing.
         *
         * @return  The quality of the drawing made (0 = nothing drawn, 100 = perfect drawing).
         **/
        virtual int drawOnto( Img<unsigned char> & im, float opacity = 1.0) = 0;


        /**
         * Return an estimation of the quality of the drawing that would be returned by calling now the
         * drawOnto() method. The default implementation return 100 is needWork return false. 
         *
         * @return  A lower bound on the quality of the current drawing. Should return >0 has soon has
         *          the image is worth drawing and 100 when the drawing is perfect.
         **/
        virtual int quality() const { if (useThreads()) { MTOOLS_ERROR("quality() should be overriden."); } return 100;  }


		/* determines if the object uses worker thread to create a drawing */
		
        /**
         * Indicate whether the object uses working threads to generate the image.
         *
         * @return  true if thread are used, false (default) otherwise.
         **/
        virtual bool useThread() const { return false; }

		
        /**
         * Enable/disable the working threads. 
         *
         * @param   status  true to enable the working threads and false to disable them. 
         **/
        void workThreads(bool status) { return; }


        /**
         * Return the status of the working threads. Return false if useThreads() return false. 
         *
         * @return  true if the working threads are currently enabled.
         **/
        bool workThreads() const; {return false; }

    };


    /**
     * Class which automates the `work()` method of a `Drawable2DObject`. This class creates an
     * independent thread which keep the drawing of the underlying Drawable2DObject updated.
     * 
     * Any Drawable2DObject can be plugged in. If the object does not need work to draw (i.e.
     * `needWork()` return false) then this object does nothing (but it still provide an interface
     * to the underlying object). On the other hand, if `needWork()` return true, the worker thread
     * is created and can be managed via the `workThread()` method.
     **/
    class AutoDrawable2DObject
    {

    public:

         /**
          * Constructor. The object start with the work thread enabled.
          *
          * @param [in,out] obj The drawable 2D object that should be managed. Its lifetime must exceed
          *                     that of this object.
          * @param  startThread true to start the worker thread if it is needed. False to prevent starting
          *                     the worker thread.
          **/
         AutoDrawable2DObject(Drawable2DObject * obj, bool startThread = true);


         /**
         * Destructor. Stop the working thread if active but does not delete the managed object.
         **/
         virtual ~AutoDrawable2DObject();


        /**
         * Sets the parameters of the drawing.
         *
         * @param   range       The range to draw.
         * @param   imageSize   The size of the desired picture.
         **/
        void setParam(mtools::fBox2 range, mtools::iVec2 imageSize);


        /**
         * Force a reset of the drawing. This method is called to indicate that the underlying object
         * drawn may have changed and every previous drawing should be discarded.
         **/
        void resetDrawing();


        /**
         * Draw onto a given image. This method is called when we want the picture. This method should
         * be as fast as possible  (eventually indicating that it is incomplete by returning a number
         * smaller than 100).
         *
         * @param [in,out]  im  The image to draw onto (must accept both 3 or 4 channel images).
         * @param   opacity     The opacity that should be applied to the picture before drawing onto im.
         *                      Hence, if opacity = 1.0, overwrite im and if opacity = 0.0 do nothing.
         *
         * @return  The quality of the drawing made (0 = nothing drawn, 100 = perfect drawing).
         **/
        int drawOnto(Img<unsigned char> & im, float opacity = 1.0);


        /**
         * Return the quality of the current drawing : a call to drawOnto should produce a drawing with
         * a quality bounded below by this value.
         *
         * @return  The quality of the drawing between 0 (nothng to show) and 100 (perfect).
         **/
        int quality() const;


        /**
         * Determines if the Object needs the working thread in order to produce a drawing. This method
         * simply transfer the result from the associated needWork() method of the underlying
         * Drawable2DObject.
         *
         * @return  true if work is needed and false otherwise.
         **/
        bool needWork() const;


        /**
         * Start / Stop the working thread. 
         *
         * @param   status  true to enable the working thread and false to disable it. 
         **/
        void workThread(bool status);


        /**
         * Return the status of the working thread. ALways return false if needWork() return false. 
         *
         * @return  true if the working thread is currently enabled.
         **/
        bool workThread() const;


    private:

        AutoDrawable2DObject(const AutoDrawable2DObject &) = delete;                // no copy.
        AutoDrawable2DObject & operator=(const AutoDrawable2DObject &) = delete;    //

        void _workerThread(); 

        void _stopThread();

        void _startThread();

        mutable std::mutex _mut;
        std::atomic<bool> _mustexit;
        std::atomic<bool> _threadon;

        Drawable2DObject * _obj; // the drawable object to manage
    };


}

}

