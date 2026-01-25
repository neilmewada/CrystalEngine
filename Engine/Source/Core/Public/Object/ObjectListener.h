#pragma once

namespace CE
{
    class Object;

    struct CORE_API IObjectUpdateListener
    {
        virtual ~IObjectUpdateListener() {}

        virtual void OnObjectFieldChanged(Uuid object, const Name& fieldName) {}
        virtual void OnObjectFieldEdited(Uuid object, const Name& fieldName) {}
    };

    class CORE_API ObjectListener
    {
        CE_STATIC_CLASS(ObjectListener)
    public:

        static void AddListener(Uuid target, IObjectUpdateListener* listener);

        static void RemoveListener(Uuid target, IObjectUpdateListener* listener);
        static void RemoveAllListeners(Uuid target);

    private:

        static void Trigger(Uuid object, const Name& fieldName);
        static void TriggerEdit(Uuid object, const Name& fieldName);

        static SharedRecursiveMutex mutex;
        static HashMap<Uuid, Array<IObjectUpdateListener*>> listeners;

        friend class Object;
    };

} // namespace CE
