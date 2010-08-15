module Swift
  class Scheme
    attr_accessor :tuple
    alias_method :scheme, :class

    def initialize options = {}
      @tuple = scheme.header.new_tuple
      options.each{|k, v| send(:"#{k}=", v)}
    end

    def update options = {}
      options.each{|k, v| send(:"#{k}=", v)}
      Swift.db.update(scheme, self)
    end

    def destroy
      Swift.db.destroy(scheme, self)
    end

    class << self
      attr_accessor :header

      def inherited klass
        klass.header = Header.new
        klass.header.push(*header) if header
        Swift.schema.push(klass)   if klass.name
      end

      def load tuple
        scheme       = allocate
        scheme.tuple = tuple
        scheme
      end

      def attribute name, type, options = {}
        header.push(attribute = type.new(self, name, options))
        (class << self; self end).send(:define_method, name, lambda{ attribute })
      end

      def store name = nil
        name ? @store = name : @store
      end

      def create options = {}
        Swift.db.create(self, options)
      end

      def get keys
        Swift.db.get(self, keys)
      end

      def all conditions = '', *binds, &block
        Swift.db.all(self, conditions, *binds, &block)
      end

      def first conditions = '', *binds, &block
        Swift.db.first(self, conditions, *binds, &block)
      end
    end
  end # Scheme
end # Swift

